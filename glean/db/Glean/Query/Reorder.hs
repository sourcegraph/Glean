module Glean.Query.Reorder
  ( reorder
  ) where

import Control.Monad.Except
import Control.Monad.State
import Data.IntMap (IntMap)
import qualified Data.IntMap as IntMap
import Data.IntSet (IntSet)
import qualified Data.IntSet as IntSet
import qualified Data.List.NonEmpty as NonEmpty
import Data.Maybe
import Data.Text (Text)
import qualified Data.Text as Text
import Data.Text.Prettyprint.Doc hiding ((<>))

import Glean.Query.BindOrder
import Glean.Query.Codegen
import Glean.Query.Flatten.Types hiding (fresh)
import Glean.Query.Vars
import Glean.RTS.Term as RTS hiding (Match(..))
import Glean.RTS.Types as RTS
import qualified Glean.Database.Schema.Types as Schema

{-
Reordering pass
---------------

INPUT

A FlattenedQuery, in which all the statements have the form

   P1 | .. | Pn  =  Q1 | .. | Qn

where each P/Q has the form (G where Statements)
where G is a generator, which is either
  - a pattern
  - a fact generator
  - an array generator
  - a call to a primitive

Patterns contain no nested generators at this point, everything has
been lifted to statements by the flattening pass.


OUTPUT

A query ready for code generation (CodegenQuery) in which all
statements have the form

   P  =  Q1 | .. | Qn

where
  - P is a pattern, Q is (Generator where Statements)

In addition, the output has valid binding. Namely:

  - MatchBind will occur before MatchVar for a given variable, where "before"
    means "left-to-right top-to-bottom".

  - There are no MatchBind or MatchWild in an expression context. These are:
    - patterns on the rhs of a statement
    - the head of the query
    - the array of an array generator
    - arguments to a primitive call


HOW?

Note first that the pass might FAIL if it can't find a way to express
the query such that it satisfies the above constraints.  For example,
there's no way to make

   _ = _

valid.  Similarly, there's no way to make

   X = Y

valid if neither X nor Y is bound by anything. However, if X or Y can
be bound by a later statement, then it might be possible to reorder
statements to make this valid.

In general, establishing correct binding could mean
  - re-ordering statements
  - flipping statements from P = Q to Q = P

Moreover, removing generators and or-patterns on the left will also
require some transformations.

For now, the pass does nothing clever: no reordering and limited
flipping. More cleverness will be added later.

-}


reorder :: Schema.DbSchema -> FlattenedQuery -> Except Text CodegenQuery
reorder dbSchema QueryWithInfo{..} =
  withExcept (\(e, _) -> Text.pack $ show $
    vcat [pretty e, nest 2 $ vcat [ "in:", pretty qiQuery]]) qi
  where
    qi = do
      (q, ReorderState{..}) <-
        flip runStateT (initialReorderState qiNumVars dbSchema) $
          reorderQuery qiQuery
      return (QueryWithInfo q roNextVar qiReturnType)

reorderQuery :: FlatQuery -> R CgQuery
reorderQuery (FlatQuery pat _ stmts) = do
  stmts' <- mapM reorderStmtGroup stmts
  pat' <- fixVars IsExpr pat
  return (CgQuery pat' (concat stmts'))

{-
Note [Optimising statement groups]

A nested fact match in Angle compiles to a group of statements. For
example

   P { _, Q "abc" }

after flattening yields the group of statements

   P { _, X }
   X = Q "abc"

The purpose of reorderStmtGroup is to find a good ordering for the
statements in the group.

Things that we take into acccount are:
- A match with at most one result (point query) should probably be done early
- A match that never fails should probably be done late
- If a statement binds something that makes another statement more
  efficient, choose the ordering to exploit that

The algorithm is:

- construct a graph where the nodes are statements and
  - for each lookup statement (X = pred pat) (A)
  - for each statement that mentions X (B)
  - if pat is irrefutable
    - edge A -> B
  - if pat is a point match
    - edge B -> A
  - else, if X occurs in a prefix position in B
    - edge B -> A
  - else
    - edge A -> B
- render the postorder traversal of this graph

-}

reorderStmtGroup :: FlatStatementGroup -> R [CgStatement]
reorderStmtGroup stmts = do
  Scope scope <- gets roScope
  let
    nodes = zip [(0::Int)..] (NonEmpty.toList stmts)

    withVars :: [(Int, VarSet, FlatStatement)]
    withVars =
      [ (n, varsOf stmt IntSet.empty `IntSet.difference` scope, stmt)
      | (n, stmt) <- nodes
      ]

    -- statements in this group that are fact lookups (X = pred pat)
    lookups =
      [ (n, x, matchType, stmt)
      | (n, xs, stmt) <- withVars
      , Just (x, matchType) <- [isLookup stmt]
      , x `IntSet.member` xs
      ]

    -- Just X if the statement will compile to a fact lookup if X is bound.
    -- This is conservative and only matches simple cases, but it's enough
    -- to spot statements generated when we flatten a nested generator.
    -- At this point we also classify the match according to whether it
    -- matches at most one thing, matches everything, or something else
    -- (PatternMatch).
    isLookup
      (FlatStatement _ (Ref v) (FactGenerator _ key _))
      | Just (Var _ x _) <- matchVar v =
        Just (x, classifyPattern lookupScope key)
    isLookup _ = Nothing

    lookupVars = IntSet.fromList [ x | (_, x, _, _) <- lookups ]

    -- for classifyPattern we want to consider the variables bound by
    -- lookups as bound, because otherwise a nested match would
    -- appear to be irrefutable.
    lookupScope = scope `IntSet.union` lookupVars

    -- for each statements in this group, find the variables that
    -- the statement mentions in a prefix position.
    uses =
      [ (n, xs, prefixVars lookupVars scope stmt, stmt)
      | (n, xs, stmt) <- withVars
      ]

    -- find the statements that mention X
    usesOf x = IntMap.lookup x m
      where
      m = IntMap.fromListWith (++)
        [ (x, [(n,ys,stmt)])
        | (n, xs, ys, stmt) <- uses
        , x <- IntSet.toList xs
        ]

    edges :: IntMap [(Int,FlatStatement)]
    edges = IntMap.fromListWith (++)
      [ case matchType of
            -- a point match: do it first
          PatternMatchesOne -> (use, [(lookup,lookupStmt)])
            -- an irrefutable match: do it later
          PatternMatchesAll -> (lookup, [(use, useStmt)])
            -- otherwise: do the inner match first only if the variable is
            -- in a prefix position (so the outer match will be faster)
          PatternMatchesSome
            | x `IntSet.member` prefix -> (use, [(lookup,lookupStmt)])
            | otherwise -> (lookup, [(use, useStmt)])
      | (lookup, x, matchType, lookupStmt) <- lookups
      , Just uses <- [usesOf x]
      , (use, prefix, useStmt) <- uses
      , use /= lookup
      ]

  -- order the statements and then recursively reorder nested groups
  concat <$> mapM (reorderStmt . snd) (postorderDfs nodes edges)

data PatternMatch
  = PatternMatchesAll
    -- ^ irrefutable (never fails to match)
  | PatternMatchesOne
    -- ^ point-match: matches at most one value
  | PatternMatchesSome
    -- ^ neither of the above

-- | Classify a pattern according to whether it's irrefutable or a
-- point-match, or neither.
classifyPattern
  :: VarSet -- ^ variables in scope
  -> Term (Match () Var)
  -> PatternMatch
classifyPattern scope t = go PatternMatchesSome t id
  where
  -- during the traversal the current PatternMatch (pref) means:
  --   PatternMatchSome: we're at the beginning
  --   PatternMatchOne: we've seen a fixed prefix so far
  --   PatternMatchAll: we've seen a wild prefix so far
  go
    :: PatternMatch
    -> Term (Match () Var)
    -> (PatternMatch -> PatternMatch)
    -> PatternMatch
  go pref t r = case t of
    Byte{} -> fixed pref r
    Nat{} -> fixed pref r
    Array xs -> termSeq pref xs r
    ByteArray{} -> fixed pref r
    Tuple xs -> termSeq pref xs r
    Alt _ t -> fixed pref (\pref -> go pref t r)
    String{} -> fixed pref r
    Ref m -> case m of
      MatchWild{} -> wild pref r
      MatchNever{} -> PatternMatchesSome
      MatchFid{} -> fixed pref r
      MatchBind (Var _ v _) -> var v
      MatchVar (Var _ v _) -> var v
      MatchAnd a b ->
        go pref a $ \resulta ->
        go pref b $ \resultb ->
        case (resulta, resultb) of
          (PatternMatchesOne, _) -> r PatternMatchesOne
          (_, PatternMatchesOne) -> r PatternMatchesOne
          (PatternMatchesSome, _) -> PatternMatchesSome -- stop here
          (_, PatternMatchesSome) -> PatternMatchesSome -- stop here
          _ -> r PatternMatchesAll
      MatchPrefix _ t -> fixed pref (\pref' -> go pref' t r)
      MatchSum{} -> PatternMatchesSome -- TODO conservative
      MatchExt{} -> PatternMatchesSome
    where
    var v
      | v `IntSet.member` scope = fixed pref r
      | otherwise = wild pref r

  -- we've seen a bit of fixed pattern
  fixed PatternMatchesAll _ = PatternMatchesSome -- stop here
  fixed PatternMatchesOne r = r PatternMatchesOne
  fixed PatternMatchesSome r = r PatternMatchesOne

  -- we've seen a bit of wild pattern
  wild PatternMatchesAll r = r PatternMatchesAll
  wild PatternMatchesOne _ = PatternMatchesSome -- stop here
  wild PatternMatchesSome r = r PatternMatchesAll

  termSeq pref [] r = r pref
  termSeq pref (x:xs) r = go pref x (\pref -> termSeq pref xs r)


-- | Determine the set of variables that occur in a prefix position in
-- a pattern.
prefixVars
  :: VarSet
     -- ^ bound variables that we're interested in
  -> VarSet
     -- ^ bound variables that we're not interested in
  -> FlatStatement
  -> VarSet
prefixVars lookups scope stmt = prefixVarsStmt stmt
  where
  prefixVarsStmt (FlatStatement _ _ (FactGenerator _ key _)) =
      prefixVarsTerm key IntSet.empty
  prefixVarsStmt (FlatStatement _ _ _) = IntSet.empty
  prefixVarsStmt (FlatDisjunction stmtsss) =
    IntSet.unions $
      [ prefixVarsStmt stmt
      | stmtss <- stmtsss
      , stmts <- stmtss
      , stmt <- NonEmpty.toList stmts
      ]

  prefixVarsTerm :: Term (Match () Var) -> VarSet -> VarSet
  prefixVarsTerm t r = case t of
    Byte{} -> r
    Nat{} -> r
    Array xs -> foldr prefixVarsTerm r xs
    ByteArray{} -> r
    Tuple xs -> foldr prefixVarsTerm r xs
    Alt _ t -> prefixVarsTerm t r
    String{} -> r
    Ref m -> prefixVarsMatch m r

  prefixVarsMatch :: Match () Var -> VarSet -> VarSet
  prefixVarsMatch m r = case m of
    MatchWild{} -> IntSet.empty
    MatchNever{} -> IntSet.empty
    MatchFid{} -> r
    MatchBind (Var _ v _) -> var v
    MatchVar (Var _ v _) -> var v
    MatchAnd a b -> prefixVarsTerm a r `IntSet.union` prefixVarsTerm b r
    MatchPrefix _ t -> prefixVarsTerm t r
    MatchSum alts -> IntSet.unions (map (`prefixVarsTerm` r) (catMaybes alts))
    MatchExt{} -> IntSet.empty
    where
    var v
      -- already bound: we're still in the prefix
      | v `IntSet.member` scope = r
      -- one of the lookups in this group: add to our list of prefix vars
      | v `IntSet.member` lookups = IntSet.insert v r
      -- unbound: this is the end of the prefix
      | otherwise = IntSet.empty

postorderDfs :: forall a. [(Int,a)] -> IntMap [(Int,a)] -> [(Int,a)]
postorderDfs nodes edges = go IntSet.empty nodes (\_ -> [])
  where
  go :: IntSet -> [(Int,a)] -> (IntSet -> [(Int,a)]) -> [(Int,a)]
  go seen [] cont = cont seen
  go seen ((n,a):xs) cont
    | n `IntSet.member` seen = go seen xs cont
    | otherwise = go (IntSet.insert n seen) children
        (\seen -> (n,a) : go seen xs cont)
    where
    children = IntMap.findWithDefault [] n edges

-- | Decide whether to flip a statement or not.
--
-- For a statement P = Q we will try both P = Q and Q = P to find a
-- form that has valid binding (no unbound variables or wildcards in
-- expressions).
--
-- There's a bit of delicacy around which one we try first. Choosing
-- the right one may lead to better code. For example:
--
--    cxx.Name "foo" = X
--
-- if X is bound, we can choose whether to flip or not. But if we
-- don't flip this, the generator on the left will be bound separately
-- by toCgStatement to give
--
--    Y = X
--    Y = cxx.Name "foo"
--
-- it would be better to flip the original statement to give
--
--    X = cxx.Name "foo"
--
-- More generally, if we have generators on the left but not the
-- right, we should probably flip.  If we have generators on both
-- sides, let's conservatively try not flipping first.
--
reorderStmt :: FlatStatement -> R [CgStatement]
reorderStmt stmt@(FlatStatement ty lhs gen)
  | Just flip <- canFlip =
    noflip `catchErrorRestore` \e ->
      flip `catchErrorRestore` \e' ->
        attemptBindFromType e noflip `catchErrorRestore` \_ ->
          attemptBindFromType e' flip `catchErrorRestore` \_ ->
            giveUp e
  -- If this statement can't be flipped, we may still need to bind
  -- unbound variables:
  | otherwise =
    noflip `catchErrorRestore` \e ->
      attemptBindFromType e noflip `catchErrorRestore` \_ ->
         giveUp e
  where
  catchErrorRestore x f = do
    state0 <- get
    let restore = put state0
    x `catchError` \e -> restore >> f e

  noflip = toCgStatement (FlatStatement ty lhs gen)
  canFlip
    | TermGenerator rhs <- gen
    = Just $ toCgStatement (FlatStatement ty rhs (TermGenerator lhs))
    | otherwise
    = Nothing

  giveUp (s, e) =
    throwError (errMsg s, e)
  errMsg s = Text.pack $ show $ vcat
    [ nest 2 $ vcat ["cannot resolve:", pretty stmt]
    , nest 2 $ vcat ["because:", pretty s]
    ]

  -- In general if we have X = Y where both X and Y are unbound (or LHS = RHS
  -- containing unbound variables on both sides) then we have no choice
  -- but to return an error message. However in the specific case that we
  -- know the type of X or Y is a predicate then we can add the statement
  -- X = p _ to bind it and retry.
  --
  -- Termination is guarenteed as we strictly decrease the number of unbound
  -- variables each time
  attemptBindFromType e f
    | (_, Just (UnboundVariable var@(Var ty _ _))) <- e
    , RTS.PredicateRep pid <- RTS.repType ty = tryBindPredicate var pid
    | otherwise =
      throwError e
    where
    tryBindPredicate var pid = do
      state <- get
      details <- case Schema.lookupPid pid $ roDbSchema state of
          Nothing ->
            lift $ throwError
              ( "internal error: bindUnboundPredicates: " <>
                  Text.pack (show pid)
              , Nothing )

          Just details@Schema.PredicateDetails{} -> do return details

      bind var
      stmts <- f `catchErrorRestore` \e' -> attemptBindFromType e' f
      let
        pid = Schema.predicatePid details
        ref = Schema.predicateRef details
        p = PidRef pid ref
        tyKey = Schema.predicateKeyType details
        tyValue = Schema.predicateValueType details
        pat =
          FactGenerator p
            (Ref (MatchWild tyKey))
            (Ref (MatchWild tyValue))
      -- V = p {key=_, value=_}
      -- LHS = RHS
      return $ CgStatement (Ref (MatchBind var)) pat : stmts

-- fallback: just convert other statements to CgStatement
reorderStmt stmt = toCgStatement stmt


toCgStatement :: FlatStatement -> R [CgStatement]
toCgStatement (FlatStatement _ lhs gen) = do
  gen' <- fixVars IsExpr gen -- NB. do this first!
  lhs' <- fixVars IsPat lhs
  return [CgStatement lhs' gen']
toCgStatement (FlatDisjunction [stmts]) =
  concat <$> mapM reorderStmtGroup stmts
toCgStatement (FlatDisjunction stmtss) = do
  cg <- intersectBindings stmtss
  return [CgDisjunction cg]
  where
  intersectBindings [] = return []
  intersectBindings rs = do
    scope0 <- gets roScope
    let
      doOne r = do
        modify $ \state -> state { roScope = scope0 }
        a <- reorderAlt r
        scope <- gets roScope
        return (a,scope)
    results <- mapM doOne rs
    let intersectScope = foldr1 IntSet.intersection (map (unScope.snd) results)
    as' <- forM results $ \(a,scope) ->
      renameAlt (unScope scope `IntSet.difference` intersectScope) a
    modify $ \state -> state { roScope = Scope intersectScope }
    return as'

  reorderAlt stmts = concat <$> mapM reorderStmtGroup stmts

  -- Rename local variables in each branch of |. See Note [local variables].
  renameAlt :: IntSet -> [CgStatement] -> R [CgStatement]
  renameAlt vars stmts = do
    let n = IntSet.size vars
    state@ReorderState{..} <- get
    put state { roNextVar = roNextVar + n }
    let env = IntMap.fromList (zip (IntSet.toList vars) [ roNextVar .. ])
    return (map (fmap (rename env)) stmts)
    where
    rename :: IntMap Int -> Var -> Var
    rename env v@(Var ty x nm) = case IntMap.lookup x env of
      Nothing -> v
      Just y -> Var ty y nm


{- Note [local variables]

This is a legit query:

  X = "a" | (X where cxx.Name X)

This looks dodgy because X only appears in one branch on the rhs, but
in fact it's fine:
* (X where cxx.Name X) is reasonable
* "a" | (X where cxx.Name X) is reasonable, but doesn't bind X
* therefore in X = "a" | (X where cxx.Name X), the X on the left is binding.

You might think "let's reject it".  But even if the user didn't write
it like this, We might end up here after query optimisation, e.g. it
can start as

  X = "a" | (Z where cxx.Name Z)

and then unification will replace [X/Z]. Should we avoid doing that?
It seems hard to avoid while still doing all the useful optimisation
we want. e.g. in

  X = cxx.Name "foo"
  { X, Y } = { N, cxx.RecordDeclaraiton { name = N } } | ...

we'd really like N to unify with X.

So, let's just make it work.  If we do the naive thing and map X to a
variable _0, the codegen will see

  _0:string = "a" | (_0 where cxx.Name _0:string)

and it will generate bogus code, because the value we build in _0 on
the left depends on a value in _0 on the right.

To fix this we need to identify variables that are local to one side
of | and rename them so they can't clash with variables mentioned
elsewhere.  A "local" variable is one that isn't bound by both
branches of |.
-}


fixVars :: FixBindOrder a => IsPat -> a -> R a
fixVars isPat p = do
  scope <- gets roScope
  (p', scope') <-
    lift $
      withExcept (\err -> (errMsg err, Just err)) $
      runFixBindOrder scope (fixBindOrder isPat p)
  modify $ \state -> state { roScope = scope' }
  return p'
  where
    errMsg err = case err of
      UnboundVariable v@(Var _ _ nm) ->
        "unbound variable: " <> fromMaybe (Text.pack $ show v) nm
      CannotUseWildcardInExpr -> "cannot use a wildcard in an expression"


data ReorderState = ReorderState
  { roNextVar :: !Int
  , roScope :: Scope
  , roDbSchema :: Schema.DbSchema
  }

type R a = StateT ReorderState (Except (Text, Maybe FixBindOrderError)) a

initialReorderState :: Int -> Schema.DbSchema -> ReorderState
initialReorderState nextVar dbSchema = ReorderState
  { roNextVar = nextVar
  , roScope = Scope IntSet.empty
  , roDbSchema = dbSchema
  }

bind :: Var -> R ()
bind (Var _ v _) = modify $ \state ->
  state { roScope = Scope (IntSet.insert v (unScope (roScope state))) }