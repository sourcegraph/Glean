-- | This is a very incomplete skeleton of an in-memory storage backend.
-- At the moment it is only usable for tests which don't produce or query
-- facts.

module Glean.Database.Storage.Memory
  ( Memory
  , newStorage
  ) where

import Control.Concurrent.STM
import Data.ByteString (ByteString)
import Data.HashMap.Strict (HashMap)
import qualified Data.HashMap.Strict as HashMap

import Glean.Database.Exception
import Glean.Database.Repo (Repo)
import Glean.Database.Storage
import Glean.RTS.Foreign.FactSet (FactSet)
import qualified Glean.RTS.Foreign.FactSet as FactSet
import Glean.RTS.Foreign.Lookup
import Glean.RTS.Types (lowestPid)
import Glean.Types (PredicateStats(..))

newtype Memory = Memory (TVar (HashMap Repo (Database Memory)))

newStorage :: IO Memory
newStorage = Memory <$> newTVarIO HashMap.empty

-- | An abstract storage for fact database
instance Storage Memory where
  data Database Memory = Database
    { dbRepo :: Repo
    , dbFacts :: FactSet
    , dbData :: TVar (HashMap ByteString ByteString)
    }

  open (Memory v) repo (Create start _) _ = do
    facts <- FactSet.new start
    atomically $ do
      dbs <- readTVar v
      case HashMap.lookup repo dbs of
        Nothing -> do
          db <- Database repo facts <$> newTVar mempty
          writeTVar v $ HashMap.insert repo db dbs
          return db
        Just _ -> dbError repo "database already exists"
  open (Memory v) repo _ _ = do
    dbs <- readTVarIO v
    case HashMap.lookup repo dbs of
      Just db -> return db
      Nothing -> dbError repo "database doesn't exist"

  -- TODO
  close _ = return ()

  delete (Memory v) = atomically . modifyTVar' v . HashMap.delete

  -- FIXME: This is a terrible hack to ensure we don't remove everything when
  -- thinning the schema
  predicateStats _ =
    return $ take 1000 [(pid, PredicateStats 1 1) | pid <- [lowestPid ..]]

  store db key value =
    atomically $ modifyTVar' (dbData db) $ HashMap.insert key value
  retrieve db key =
    atomically $ HashMap.lookup key <$> readTVar (dbData db)

  commit = FactSet.append . dbFacts
  optimize _ = return ()
  -- TODO
  backup db _ _ = dbError (dbRepo db) "unimplemented 'backup'"
  -- TODO
  restore _ repo _ _ = dbError repo "unimplemented 'restore'"

instance CanLookup (Database Memory) where
  withLookup = withLookup . dbFacts