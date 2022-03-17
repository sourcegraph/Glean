/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <boost/variant.hpp>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Sema/SemaConsumer.h>
#include <llvm/Config/llvm-config.h>

#include "folly/MapUtil.h"
#include "folly/ScopeGuard.h"
#include "glean/lang/clang/ast.h"
#include "glean/lang/clang/index.h"

// This file implements the Clang AST traversal.

namespace {

using namespace facebook::glean::clangx;
using namespace facebook::glean::cpp;

template<typename T> T identity(T x) { return x; }

/// Track usage of using declarations
class UsingTracker {
public:
  explicit UsingTracker(const clang::DeclContext *context, ClangDB& d)
    : globalContext(getCanonicalDeclContext(context))
    , currentContext(globalContext)
    , db(d)
  {
    CHECK_NOTNULL(currentContext);
  }

  void addNamespace(const clang::NamespaceDecl *decl) {
    if (decl->isAnonymousNamespace() || decl->isInline()) {
      auto parent = getCanonicalDeclContext(decl->getParent());
      auto context = getCanonicalDeclContext(decl);
      forwards[parent].push_back(
        Forward{decl->getBeginLoc(), context, folly::none});

    }
  }

  /// Add a UsingDecl to the current DeclContext
  void addUsingDecl(
      const clang::UsingDecl *decl, Fact<Cxx::UsingDeclaration> fact) {
    // TODO: We don't handle class-scope UsingDecls for now as we'd have to
    // deal with inheritance for that.
    if (!clang::isa<clang::RecordDecl>(decl->getDeclContext())) {
      const auto context = getCanonicalDeclContext(decl->getDeclContext());
      for (const auto *shadow : decl->shadows()) {
        if (const auto target = shadow->getTargetDecl()) {
          if (auto canonical = clang::dyn_cast<clang::NamedDecl>(
              target->getCanonicalDecl())) {
            if (auto tpl = clang::dyn_cast<clang::RedeclarableTemplateDecl>(
                  canonical)) {
              canonical = tpl->getTemplatedDecl();
            }
            usingDecls.insert({{context, canonical}, {decl, fact}});
          }
        }
      }
    }
  }

  void addUsingDirective(
      const clang::UsingDirectiveDecl *decl, Fact<Cxx::UsingDirective> fact) {
    if (auto context = getCanonicalDeclContext(decl->getDeclContext())) {
      if (auto dir_context =
            getCanonicalDeclContext(
              clang::dyn_cast<clang::DeclContext>(
                decl->getNominatedNamespace()))) {
        forwards[context].push_back(
          Forward{decl->getUsingLoc(), dir_context, fact});
      }
    }
  }


  /// Given a NamedDecl and its XRefTarget, add XRefTarget.indirect if the xref
  /// goes through using declarations
  Cxx::XRefTarget retarget(
      const clang::Decl * FOLLY_NULLABLE base,
      Cxx::XRefTarget target) {
    if (base) {
      if (const auto decl =
            clang::dyn_cast_or_null<clang::NamedDecl>(
              base->getCanonicalDecl())) {
        const auto declContext =
          getCanonicalDeclContext(decl->getDeclContext());
        if (declContext != currentContext) {
          const clang::DeclContext *parentContext = nullptr;
          // For non-scoped enumerators, we need to take into account that
          // using the enclosing namespace of the enum type also brings the
          // enumerators into scope:
          //
          // namespace N1 { enum E { A } };
          // namespace N2 { using namespace N1; /* A is in scope */ }
          // namespace N3 { using N1::E; /* A isn't in scope */ }
          //
          // So we have to look for the DeclContext of the enum (but not for
          // the enum itself).
          if (auto enm =
                clang::dyn_cast_or_null<clang::EnumDecl>(declContext)) {
            if (!enm->isScoped()) {
              parentContext = getCanonicalDeclContext(
                enm->getCanonicalDecl()->getDeclContext());
            }
          }
          if (parentContext != currentContext) {
            LookupState state{decl, declContext, parentContext, {}, {}};
            lookup(currentContext, state);
            for (const auto& via : state.via) {
              target = Cxx::XRefTarget::indirect(
                db.fact<Cxx::XRefIndirectTarget>(via, target));
            }
          }
        }
      }
    }
    return target;
  }

private:
  struct LookupState {
    /// The canonical decl we're looking for
    const clang::NamedDecl *decl;

    /// The canonical context of the decl
    const clang::DeclContext *context;

    /// The context of the parent enum decl if we are looking for an enumerator
    const clang::DeclContext * FOLLY_NULLABLE parentContext;


    /// List of XRefVia populated by the lookup
    std::list<Cxx::XRefVia> via;

    /// Visited contexts populated by the lookup
    folly::F14FastSet<const clang::DeclContext *> visited;
  };

  folly::Optional<const clang::DeclContext *> lookupIn(
      const clang::DeclContext * context, LookupState& s) {
    if (s.visited.find(context) == s.visited.end()) {
      s.visited.insert(context);
      if (context == s.context || context == s.parentContext) {
        return nullptr;
      } else if (auto r = folly::get_optional(usingDecls, {context, s.decl})) {
        s.via.push_front(Cxx::XRefVia::usingDeclaration(r->second));
        // Follow chains of using decls - for instance:
        //
        // namespace foo { using std::vector; }
        // namespace bar { using foo::vector; }
        // bar::vector
        //
        // TODO: We should probably cache this rather than recomputing every
        // time.
        return getSpecifierContext(r->first->getQualifier());
      } else {
        auto p = forwards.find(context);
        if (p != forwards.end()) {
          // If we find a decl through a using directive we'll have to place it
          // in the right spot in the via list so remember the current position.
          auto pos = s.via.begin();
          for (auto i = p->second.rbegin(); i != p->second.rend(); ++i) {
            if (auto ctx = lookupIn(i->context, s)) {
              if (i->fact) {
                s.via.insert(
                  pos, Cxx::XRefVia::usingDirective(i->fact.value()));
              }
              return ctx;
            }
          }
        }
        return folly::none;
      }
    } else {
      return folly::none;
    }
  }

  void lookup(const clang::DeclContext *context, LookupState& s) {
    while (context) {
      if (s.visited.find(context) != s.visited.end()) {
        // We might have visited this context via a using directive so we still
        // need to go up. Example:
        //
        // namespace parent {
        // namespace child {
        //   using namespace parent;
        // }
        // }
        context = context->getLookupParent();
      } else if (auto ctx = lookupIn(context, s)) {
        context = ctx.value();
      } else {
        context = context->getLookupParent();
      }
      if (context) {
        context = getCanonicalDeclContext(context);
      }
    }
  }

public:
  // Execute f in a new DeclContext
  template<typename F>
  inline
  auto inContext(const clang::DeclContext * FOLLY_NULLABLE context, F&& f) {
    context = getCanonicalDeclContext(context);
    if (context) {
      std::swap(context, currentContext);
    }
    SCOPE_EXIT {
      if (context) {
        currentContext = context;
      }
    };
    return f();
  }

  // Execute f in the context of a NestedNameSpecifier. Consider:
  //
  // namespace foo { struct T { typedef int U; }; }
  // namespace bar { using foo::T; }
  // bar::T::U x;
  //
  // Here, the xref to U won't go through a using declaration but the xref to T
  // will so we need to make sure the xref is computed in the DeclContext of
  // bar.
  template<typename F>
  inline
  auto inNameContext(
      const clang::NestedNameSpecifier * FOLLY_NULLABLE spec, F&& f) {
    // Consider:
    //
    // namespace foo { struct T { typedef int U; }; }
    // namespace bar { using foo::T; T::U x; }
    //
    // Here, we need to make sure that the xref to T is computed in the
    // enclosing DeclContext but we can't get that from the NestedNameSpecifier.
    // So when we enter a NestedNameSpecified (the first call to inNameContext,
    // savedContext == nullptr), we store the enclosingContext in savedContext.
    // Then, when NestedNameSpecifier is null (i.e., we're about to visit the
    // left-most name), we grab the original context from savedContext.
    //
    // TODO: This is very hacky, make it better.
    //
    auto saved = savedContext;
    SCOPE_EXIT {
      savedContext = saved;
    };
    if (savedContext == nullptr) {
      savedContext = currentContext;
    }
    return inContext(spec ? getSpecifierContext(spec) : savedContext, f);
  }

  // We can't traverse the entire declaration in the context of the
  // function as this is the wrong context for the return type and for the
  // function's qualified name. So just remember the current function
  // and change the context when traversing the body (via maybeBody).
  template<typename F>
  inline
  auto inFunction(const clang::FunctionDecl *fun, F&& f) {
    auto saved = currentFunction;
    currentFunction = fun;
    SCOPE_EXIT {
      currentFunction = saved;
    };
    return f();
  }

  // Traverse function bodies in the context of the function.
  //
  // Note that the body isn't necessarily the first statement we see after
  // the function decl:
  //
  // Foo::Foo() : bar([] { ... }) { ... }
  template<typename F>
  inline
  auto maybeBody(const clang::Stmt *body, F&& f) {
    if (currentFunction && body == currentFunction->getBody()) {
      return inContext(currentFunction, std::forward<F>(f));
    } else {
      return f();
    }
  }

  const clang::DeclContext * FOLLY_NULLABLE getSpecifierContext(
      const clang::NestedNameSpecifier * FOLLY_NULLABLE spec) {
    if (spec) {
      if (auto ns = spec->getAsNamespace()) {
        return ns;
      } else if (auto rec = spec->getAsRecordDecl()) {
        return rec;
      } else if (spec->getKind()
                  == clang::NestedNameSpecifier::SpecifierKind::Global) {
        return globalContext;
      }
    }
    return nullptr;
  }

private:
  static const clang::DeclContext * FOLLY_NULLABLE getCanonicalDeclContext(
      const clang::DeclContext * FOLLY_NULLABLE ctx) {
    if (ctx) {
      return clang::dyn_cast<clang::DeclContext>(
        clang::dyn_cast<clang::Decl>(ctx)->getCanonicalDecl());
    } else {
      return nullptr;
    }
  }

  const clang::DeclContext * const globalContext;
  const clang::DeclContext *currentContext;
  const clang::DeclContext * FOLLY_NULLABLE savedContext = nullptr;
  const clang::FunctionDecl * FOLLY_NULLABLE currentFunction = nullptr;
  ClangDB& db;
  folly::F14FastMap<
    std::pair<const clang::DeclContext *, const clang::NamedDecl *>,
    std::pair<const clang::UsingDecl *, Fact<Cxx::UsingDeclaration>>>
      usingDecls;

  struct Forward {
    const clang::SourceLocation loc;
    const clang::DeclContext *context;
    folly::Optional<Fact<Cxx::UsingDirective>> fact;
  };

  folly::F14FastMap<const clang::DeclContext *, std::vector<Forward>> forwards;
};

struct ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
  using Base = clang::RecursiveASTVisitor<ASTVisitor>;

  // Memoization of a function
  template<
    typename Key,
    typename Value,
    Value (ASTVisitor::*Compute)(Key),
    Key (*Transform)(Key) = &identity<Key>>
  struct Memo {
    using key_type = Key;
    using value_type = Value;

    explicit Memo(ASTVisitor& v) : visitor(v) {}

    Value operator()(Key key) {
      auto real_key = Transform(key);
      if (auto r = folly::get_optional(items, real_key)) {
        return r.value();
      } else {
        auto value = (visitor.*Compute)(real_key);
        items.emplace(real_key, value);
        return value;
      }
    }

    folly::F14FastMap<Key, Value> items;
    ASTVisitor& visitor;
  };

  // Memoization of a function which can fail to produce a result
  template<typename Key, typename Value>
  struct MemoOptional {
    using key_type = Key;
    using value_type = Value;

    explicit MemoOptional(const std::string& t, ASTVisitor& v)
      : visitor(v), tag(t) {}

    template<typename K>
    folly::Optional<Value> operator()(K key) {
      if (auto r = folly::get_optional(items, key)) {
        auto s = r.value();
        if (s) {
          return s.value();
        } else {
          LOG(FATAL) << "MemoOptional (" << tag << "): infinite loop";
        }
      } else {
        items.insert({key, folly::none});
        auto v = Value::compute(visitor, key);
        if (v) {
          items[key] = v;
        } else {
          items.erase(key);
        }
        return v;
      }
    }

    folly::F14FastMap<Key, folly::Optional<Value>> items;
    ASTVisitor& visitor;
    const std::string tag;
  };

  // Obtain the FunctionName for a decl unless we choose to ignore it
  folly::Optional<Fact<Cxx::FunctionName>> functionName(
      const clang::NamedDecl *decl) {
    auto name = decl->getDeclName();
    switch (name.getNameKind()) {
      case clang::DeclarationName::Identifier:
        if (auto ide = name.getAsIdentifierInfo()) {
          return db.fact<Cxx::FunctionName>(
            alt<0>(db.name(ide->getName())));
        } else {
          return folly::none;
        }

      case clang::DeclarationName::ObjCZeroArgSelector:
        return folly::none;

      case clang::DeclarationName::ObjCOneArgSelector:
        return folly::none;

      case clang::DeclarationName::ObjCMultiArgSelector:
        return folly::none;

      case clang::DeclarationName::CXXOperatorName:
        return db.fact<Cxx::FunctionName>(
          alt<1>(name.getAsString()));

      case clang::DeclarationName::CXXLiteralOperatorName:
        return db.fact<Cxx::FunctionName>(
          alt<2>(name.getAsString()));

      case clang::DeclarationName::CXXConstructorName:
        return db.fact<Cxx::FunctionName>(
          alt<3>(std::make_tuple()));

      case clang::DeclarationName::CXXDestructorName:
        return db.fact<Cxx::FunctionName>(
          alt<4>(std::make_tuple()));

      case clang::DeclarationName::CXXConversionFunctionName:
        return db.fact<Cxx::FunctionName>(
          alt<5>(type(name.getCXXNameType())));

      case clang::DeclarationName::CXXDeductionGuideName:
        return folly::none;

      case clang::DeclarationName::CXXUsingDirective:
        return folly::none;
    }
  }


  /**********
   * Scopes *
   **********/


  // Scopes: global, namespace, class + access
  struct GlobalScope {};

  struct NamespaceScope {
    const clang::NamespaceDecl *decl;
    Fact<Cxx::NamespaceQName> fact;
  };

  struct ClassScope {
    const clang::RecordDecl *decl;
    Fact<Cxx::QName> fact;
  };

  struct LocalScope {
    const clang::FunctionDecl *decl;
    Fact<Cxx::FunctionQName> fact;
  };

  using Scope = boost::variant<
    GlobalScope,
    NamespaceScope,
    ClassScope,
    LocalScope>;

  // Translate clang::AccessSpecifier to cxx.Access
  static Cxx::Access access(clang::AccessSpecifier spec) {
    switch (spec) {
      case clang::AS_public:
        return Cxx::Access::Public;

      case clang::AS_protected:
        return Cxx::Access::Protected;

      case clang::AS_private:
        return Cxx::Access::Private;

      default:
        // TODO: is this always public?
        return Cxx::Access::Public;
    }
  }

  // Obtain the scope for a DeclContext
  Scope defineScope(const clang::DeclContext *ctx) {
    while (ctx) {
      if (clang::isa<clang::TranslationUnitDecl>(ctx)) {
        return GlobalScope{};
      } else if (auto x = clang::dyn_cast<clang::NamespaceDecl>(ctx)) {
        if (auto r = namespaces(x)) {
          return NamespaceScope{x, r->qname};
        }
      } else if (auto x = clang::dyn_cast<clang::CXXRecordDecl>(ctx)) {
        if (auto r = classDecls(x)) {
          return ClassScope{x, r->qname};
        }
      } else if (auto x = clang::dyn_cast<clang::FunctionDecl>(ctx)) {
        if (auto r = funDecls(x)) {
          return LocalScope{x, r->qname};
        }
      }
      ctx = ctx->getParent();
    }
    return GlobalScope{};
  }

  // Obtain the parent scope of a Decl.
  Scope parentScope(const clang::Decl *decl) {
    return scopes(decl->getDeclContext());
  }

  Cxx::Scope scopeRepr(const Scope& scope, clang::AccessSpecifier acs) {
    struct GetScope
      : boost::static_visitor<Cxx::Scope> {

      const clang::AccessSpecifier access_;

      explicit GetScope(clang::AccessSpecifier a) : access_(a) {}

      result_type operator()(const GlobalScope&) const {
        return Cxx::Scope::global_();
      }

      result_type operator()(const NamespaceScope& ns) const {
        return Cxx::Scope::namespace_(ns.fact);
      }

      result_type operator()(const ClassScope& cls) const {
        return Cxx::Scope::recordWithAccess(cls.fact, access(access_));
      }

      result_type operator()(const LocalScope& fun) const {
        return Cxx::Scope::local(fun.fact);
      }
    };

    return boost::apply_visitor(GetScope(acs), scope);
  }

  // Obtain the cxx.Scope of a Decl and translate it to clang.Scope
  Cxx::Scope parentScopeRepr(const clang::Decl *decl) {
    return scopeRepr(parentScope(decl), decl->getAccess());
  }


  struct DeclTraits {
    template<typename T>
    static bool isDefinition(const T* decl) {
      return DeclTraits::getDefinition(decl) == decl;
    }

    static bool isDefinition(const clang::ObjCMethodDecl *decl) {
      return decl->isThisDeclarationADefinition();
    }

    template<typename T>
    static const T *getDefinition(const T *decl) {
      return decl->getDefinition();
    }

    static const clang::NamespaceDecl * FOLLY_NULLABLE getDefinition(
        const clang::NamespaceDecl *) {
      return nullptr;
    }

    static const clang::TypedefNameDecl * FOLLY_NULLABLE getDefinition(
        const clang::TypedefNameDecl *) {
      return nullptr;
    }

    static const clang::FieldDecl * FOLLY_NULLABLE getDefinition(
        const clang::FieldDecl *) {
      return nullptr;
    }

    static const clang::ObjCCategoryDecl *getDefinition(
        const clang::ObjCCategoryDecl *decl) {
      return decl;    // TODO: is this right?
    }

    static const clang::ObjCImplementationDecl *getDefinition(
        const clang::ObjCImplementationDecl *decl) {
      return decl;    // TODO: is this right?
    }

    static const clang::ObjCCategoryImplDecl *getDefinition(
        const clang::ObjCCategoryImplDecl *decl) {
      return decl;    // TODO: is this right?
    }

    static const clang::ObjCMethodDecl * FOLLY_NULLABLE getDefinition(
        const clang::ObjCMethodDecl *) {
      return nullptr;
    }

    static const clang::ObjCPropertyDecl * FOLLY_NULLABLE getDefinition(
        const clang::ObjCPropertyDecl *) {
      return nullptr;
    }

    template<typename T>
    static const T *getCanonicalDecl(const T *decl) {
      return decl->getCanonicalDecl();
    }

    static const clang::ObjCCategoryDecl *getCanonicalDecl(
        const clang::ObjCCategoryDecl *decl) {
      return decl;    // TODO: is this right?
    }

    static const clang::ObjCImplementationDecl *getCanonicalDecl(
        const clang::ObjCImplementationDecl *decl) {
      return decl;    // TODO: is this right?
    }

    static const clang::ObjCCategoryImplDecl *getCanonicalDecl(
        const clang::ObjCCategoryImplDecl *decl) {
      return decl;    // TODO: is this right?
    }

    static const clang::ObjCPropertyDecl *getCanonicalDecl(
        const clang::ObjCPropertyDecl *decl) {
      return decl;    // TODO: is this right?
    }

    template<typename T>
    static constexpr bool canHaveComments(const T *) {
      return true;
    }

    static constexpr bool canHaveComments(const clang::NamespaceDecl *) {
      // Clang assigns comments at the top level of a module to the first
      // namespace decls. Comments on namespaces probably aren't interesting,
      // anyway.
      return false;
    }
  };

  template<typename Memo, typename Decl>
  folly::Optional<typename Memo::value_type> representative(
      Memo& memo,
      const Decl *decl,
      folly::Optional<typename Memo::value_type> me) {
    auto defn = DeclTraits::getDefinition(decl);
    if (defn == decl) {
      return me;
    } else if (defn != nullptr) {
      if (auto r = memo(defn)) {
        return r.value();
      }
    }

    auto can = DeclTraits::getCanonicalDecl(decl);
    if (can != nullptr) {
      if (auto r = memo(can)) {
        return r.value();
      }
    }

    return me;
  }

  template<typename Decl>
  struct Declare {
    template<typename ClangDecl>
    static folly::Optional<Decl> compute(
        ASTVisitor& visitor,
        const ClangDecl *decl) {
      auto range = visitor.db.srcRange(decl->getSourceRange());
      folly::Optional<Decl> result =
        Decl::declare(
          visitor, decl, visitor.parentScopeRepr(decl), range.range);
      if (result) {
        visitor.db.declaration(range, result->declaration());
        // The name location retrieval logic should be consistent with ClangD:
        // https://github.com/llvm/llvm-project/blob/a3a2239aaaf6860eaee591c70a016b7c5984edde/clang-tools-extra/clangd/AST.cpp#L167-L172
        auto nameRange =
            visitor.db.srcRange(visitor.db.rangeOfToken(decl->getLocation()));
        visitor.db.fact<Cxx::DeclarationNameSpan>(
            result->declaration(), nameRange.range.file, nameRange.span);
        if (DeclTraits::canHaveComments(decl)){
          if (auto comment =
                decl->getASTContext().getRawCommentForDeclNoCache(decl)) {
            auto crange = visitor.db.srcRange(comment->getSourceRange());
            visitor.db.fact<Cxx::DeclarationComment>(
              result->declaration(),
              crange.range.file,   // might be "<builtin>"
              crange.span);
          }
        }
      }
      return result;
    }
  };

  template<typename Memo, typename Decl>
  void visitDeclaration(Memo& memo, const Decl *decl) {
    if (auto cdecl = memo(decl)) {
      if (DeclTraits::isDefinition(decl)) {
        cdecl->define(*this, decl);
      }
      auto same = representative(memo, decl, cdecl.value());
      if (same) {
        const auto this_decl = cdecl->declaration();
        const auto other_decl = same->declaration();
        if (this_decl != other_decl) {
          db.fact<Cxx::Same>(this_decl, other_decl);
        }
      }
    }
  }

  /**************
   * Namespaces *
   **************/

  // Obtain the parent namespace of a Decl, if any
  folly::Optional<Fact<Cxx::NamespaceQName>> parentNamespace(
      const clang::Decl* decl) {
    struct GetNamespace
      : boost::static_visitor<folly::Optional<Fact<Cxx::NamespaceQName>>> {
      result_type operator()(const GlobalScope&) const {
        return folly::none;
      }

      result_type operator()(const NamespaceScope& ns) const {
        return ns.fact;
      }

      result_type operator()(const ClassScope&) const {
        LOG(ERROR) << "Inner scope is a class, should have been a namespace";
        return folly::none;
      }

      result_type operator()(const LocalScope&) const {
        LOG(ERROR) << "Inner scope is a function, should have been a namespace";
        return folly::none;
      }
    };

    return boost::apply_visitor(GetNamespace(), parentScope(decl));
  }

  struct NamespaceDecl : Declare<NamespaceDecl> {
    Fact<Cxx::NamespaceQName> qname;
    Fact<Cxx::NamespaceDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::namespace_(decl);
    }

    static folly::Optional<NamespaceDecl> declare(
        ASTVisitor& visitor,
        const clang::NamespaceDecl *decl,
        Cxx::Scope,
        Src::Range range) {
      folly::Optional<Fact<Cxx::Name>> name;
      if (!decl->isAnonymousNamespace()) {
        name = visitor.db.name(decl->getName());
      }

      auto qname = visitor.db.fact<Cxx::NamespaceQName>(
        maybe(name), maybe(visitor.parentNamespace(decl)));

      return NamespaceDecl
        { {}
        , qname
        , visitor.db.fact<Cxx::NamespaceDeclaration>(qname, range)
        };
    }
  };

  // Clang namespace visitor
  bool VisitNamespaceDecl(clang::NamespaceDecl *decl) {
    if (auto r = namespaces(decl)) {
      // FIXME: complete
      db.fact<Cxx::NamespaceDefinition>(
        r->decl,
        db.fact<Cxx::Declarations>(
          std::vector<Cxx::Declaration>()));
    }
    usingTracker.addNamespace(decl);
    return true;
  }


  /**********************
   * Using declarations *
   **********************/

  bool VisitUsingDecl(const clang::UsingDecl *decl) {
    if (auto name = functionName(decl)) {
      if (auto context =
          usingTracker.getSpecifierContext(decl->getQualifier())) {
        auto range = db.srcRange(decl->getSourceRange());
        auto fact = db.fact<Cxx::UsingDeclaration>(
          db.fact<Cxx::FunctionQName>(
            name.value(),
            scopeRepr(scopes(context), decl->getAccess())),
          range.range);
        db.declaration(range, Cxx::Declaration::usingDeclaration(fact));
        usingTracker.addUsingDecl(decl, fact);
      }
    }
    return true;
  }

  /********************
   * Using directives *
   ********************/

  bool VisitUsingDirectiveDecl(const clang::UsingDirectiveDecl *decl) {
    if (auto nominated = decl->getNominatedNamespace()) {
      usingTracker.inNameContext(decl->getQualifier(), [&] {
        xrefTarget(
          db.rangeOfToken(decl->getIdentLocation()),
          XRef::toDecl(namespaces, nominated)); });
    }

    if (auto ns = decl->getNominatedNamespaceAsWritten()) {
      auto range = db.srcRange(decl->getSourceRange());
      auto fact = db.fact<Cxx::UsingDirective>(
          db.fact<Cxx::QName>(
            db.name(ns->getName()),
            parentScopeRepr(ns)),
          range.range);
      db.declaration(range, Cxx::Declaration::usingDirective(fact));
      usingTracker.addUsingDirective(decl, fact);
    }
    return true;
  }

  /***********
   * Enums *
   ***********/

  Fact<Cxx::Enumerator> enumerator(
      Fact<Cxx::EnumDeclaration> type,
      const clang::EnumConstantDecl *decl) {
    return db.fact<Cxx::Enumerator>(
      db.name(decl->getName()),
      type,
      db.srcRange(decl->getSourceRange()).range);
  }

  struct EnumDecl : Declare<EnumDecl> {
    Fact<Cxx::EnumDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::enum_(decl);
    }

    static folly::Optional<EnumDecl> declare(
        ASTVisitor& visitor,
        const clang::EnumDecl *decl,
        Cxx::Scope scope,
        Src::Range range) {
      auto qname = visitor.db.fact<Cxx::QName>(
        visitor.db.name(decl->getName()), scope);

      folly::Optional<Fact<Cxx::Type>> underlying;
      if (auto ty = decl->getIntegerTypeSourceInfo()) {
        underlying = visitor.type(ty->getType());
      }

      return EnumDecl
        { {}
        , visitor.db.fact<Cxx::EnumDeclaration>(
            qname,
            decl->isScoped(),
            maybe(underlying),
            range)
        };
    }

    void define(ASTVisitor& visitor, const clang::EnumDecl *d) const {
      std::vector<Fact<Cxx::Enumerator>> enumerators;
      for (const auto& e : d->enumerators()) {
        enumerators.push_back(visitor.enumerator(decl, e));
      }

      visitor.db.fact<Cxx::EnumDefinition>(decl, enumerators);
    }
  };

  struct EnumeratorDecl {
    Fact<Cxx::Enumerator> fact;

    static folly::Optional<EnumeratorDecl> compute(
        ASTVisitor& visitor,
        const clang::EnumConstantDecl *decl) {
      if (auto ty = clang::dyn_cast<clang::EnumDecl>(decl->getDeclContext())) {
        if (auto enm = visitor.enumDecls(ty)) {
          return EnumeratorDecl{ visitor.enumerator(enm->decl, decl) };
        }
      }
      return folly::none;
    }
  };

  bool VisitEnumDecl(const clang::EnumDecl *decl) {
    visitDeclaration(enumDecls, decl);
    return true;
  }


  /****************
   * Type aliases *
   ****************/

  struct TypeAliasDecl : Declare<TypeAliasDecl> {
    Fact<Cxx::TypeAliasDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::typeAlias(decl);
    }

    static folly::Optional<TypeAliasDecl> declare(
        ASTVisitor& visitor,
        const clang::TypedefNameDecl *decl,
        Cxx::Scope scope,
        Src::Range range) {
      folly::Optional<Cxx::TypeAliasKind> kind;
      if (clang::isa<clang::TypeAliasDecl>(decl)) {
        kind = Cxx::TypeAliasKind::Using;
      } else if (clang::isa<clang::TypedefDecl>(decl)) {
        kind = Cxx::TypeAliasKind::Typedef;
      }
      if (kind) {
        auto qname = visitor.db.fact<Cxx::QName>(
          visitor.db.name(decl->getName()), scope);
        auto type = visitor.type(decl->getUnderlyingType());
        return TypeAliasDecl
          { {}
          , visitor.db.fact<Cxx::TypeAliasDeclaration>(
              qname,
              type,
              kind.value(),
              range)
          };
      } else {
        return folly::none;
      }
    }

    void define(ASTVisitor&, const clang::TypedefNameDecl *) const {}
  };

  bool VisitTypedefNameDecl(const clang::TypedefNameDecl *decl) {
    visitDeclaration(typeAliasDecls, decl);
    return true;
  }

  /***********
   * Classes *
   ***********/

  struct ClassDecl : Declare<ClassDecl> {
    Fact<Cxx::QName> qname;
    Fact<Cxx::RecordDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::record_(decl);
    }

    static folly::Optional<ClassDecl> declare(
        ASTVisitor& visitor,
        const clang::CXXRecordDecl *decl,
        Cxx::Scope scope,
        Src::Range range) {
      if (decl->isInjectedClassName()) {
        return folly::none;
      }

      folly::Optional<Cxx::RecordKind> kind;
      switch (decl->getTagKind()) {
        case clang::TTK_Struct:
          kind = Cxx::RecordKind::struct_();
          break;

        case clang::TTK_Class:
          kind = Cxx::RecordKind::class_();
          break;

        case clang::TTK_Union:
          kind = Cxx::RecordKind::union_();
          break;

        default:
          break;
      }
      if (kind) {
        auto qname = visitor.db.fact<Cxx::QName>(
          visitor.db.name(decl->getName()), scope);
        return ClassDecl
          { {}
          , qname
          , visitor.db.fact<Cxx::RecordDeclaration>(
              qname,
              kind.value(),
              range)
          };
      } else {
        return folly::none;
      }
    }

    void define(ASTVisitor& visitor, const clang::CXXRecordDecl *d) const {
      std::vector<Cxx::RecordBase> bases;
      for (const auto& base : d->bases()) {
        if (auto ty = base.getType().getTypePtrOrNull()) {
          if (auto record = ty->getAsCXXRecordDecl()) {
            if (auto other = visitor.classDecls(record)) {
              bases.push_back(Cxx::RecordBase{
                other->decl,  // should this be base.representative?
                visitor.access(base.getAccessSpecifier()),
                base.isVirtual()
              });
            }
          }
        }
      }

      std::vector<Cxx::Declaration> members;
      for (const auto& mem : d->decls()) {
        if (auto record = clang::dyn_cast<clang::CXXRecordDecl>(mem)) {
          if (!record->isInjectedClassName()) {
            if (auto m = visitor.classDecls(record)) {
              members.push_back(m->declaration());
            }
          }
        } else if (auto fun = clang::dyn_cast<clang::FunctionDecl>(mem)) {
          // Skip implicit constructors/destructors
          if (fun->isImplicit()) continue;
          if (auto m = visitor.funDecls(fun)) {
            members.push_back(Cxx::Declaration::function_(m->decl));
          }
        } else if (auto ed = clang::dyn_cast<clang::EnumDecl>(mem)) {
          if (auto m = visitor.enumDecls(ed)) {
            members.push_back(Cxx::Declaration::enum_(m->decl));
          }
        } else if (auto vd = clang::dyn_cast<clang::VarDecl>(mem)) {
          if (auto m = visitor.varDecls(vd)) {
            members.push_back(Cxx::Declaration::variable(m->decl));
          }
        } else if (auto tad = clang::dyn_cast<clang::TypeAliasDecl>(mem)) {
          if (auto m = visitor.typeAliasDecls(tad)) {
            members.push_back(Cxx::Declaration::typeAlias(m->decl));
          }
        }
      }
      visitor.db.fact<Cxx::RecordDefinition>(
        decl,
        bases,
        visitor.db.fact<Cxx::Declarations>(members));
    }

    static const clang::CXXRecordDecl * FOLLY_NULLABLE getInstantiatedMember(
        const clang::CXXRecordDecl *decl) {
      return decl->getInstantiatedFromMemberClass();
    }

    static const clang::CXXRecordDecl * FOLLY_NULLABLE getSpecializedDecl(
        const clang::CXXRecordDecl *decl) {
      if (auto spec =
            clang::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(
              decl)) {
        auto tpl = spec->getSpecializedTemplateOrPartial();
        if (auto tdecl = tpl.dyn_cast<clang::ClassTemplateDecl*>()) {
          return tdecl->getTemplatedDecl();
        } else if (auto tspec =
            tpl.dyn_cast<clang::ClassTemplatePartialSpecializationDecl*>()) {
          return tspec;
        }
      }
      return nullptr;
    }
  };

  // Clang record visitor
  bool VisitCXXRecordDecl(const clang::CXXRecordDecl *decl) {
    visitDeclaration(classDecls, decl);
    return true;
  }

  /*************
   * Functions *
   *************/

  Fact<Cxx::Type> type(const clang::QualType& ty) {
    return db.fact<Cxx::Type>(ty.getAsString(astContext.getPrintingPolicy()));
  }

  Fact<Cxx::Signature> signature(
      const clang::QualType& result,
      clang::ArrayRef<clang::ParmVarDecl *> parameters) {
    std::vector<Cxx::Parameter> params;
    for (auto parm : parameters) {
      params.push_back({db.name(parm->getName()), type(parm->getType())});
    }

    return db.fact<Cxx::Signature>(type(result), params);
  }

  static Cxx::RefQualifier refQualifier(clang::RefQualifierKind rq) {
    switch (rq) {
      case clang::RQ_None:
        return Cxx::RefQualifier::None_;

      case clang::RQ_LValue:
        return Cxx::RefQualifier::LValue;

      case clang::RQ_RValue:
        return Cxx::RefQualifier::RValue;

      default:
        LOG(ERROR) << "unknown clang::RefQualifiedKind";
        return Cxx::RefQualifier::None_;
    }
  }

  struct FunDecl : Declare<FunDecl> {
    Fact<Cxx::FunctionQName> qname;
    Fact<Cxx::FunctionDeclaration> decl;
    bool method;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::function_(decl);
    }

    static folly::Optional<FunDecl> declare(
        ASTVisitor& visitor,
        const clang::FunctionDecl *decl,
        Cxx::Scope scope,
        Src::Range range) {
      // TODO: should we ignore deleted functions or have some info about them?
      if (decl->isDeleted()
        || decl->getTemplateSpecializationKind()
            == clang::TSK_ImplicitInstantiation) {
        return folly::none;
      }

      if (auto name = visitor.functionName(decl)) {
        folly::Optional<Cxx::MethodSignature> method;

        if (auto mtd = clang::dyn_cast<clang::CXXMethodDecl>(decl)) {
          if (mtd->isInstance()) {
            method = Cxx::MethodSignature{
              mtd->isVirtual(),
              mtd->isConst(),
              mtd->isVolatile(),
              visitor.refQualifier(mtd->getRefQualifier())
            };
          }
        }

        auto qname = visitor.db.fact<Cxx::FunctionQName>(name.value(), scope);
        auto decl_fact = visitor.db.fact<Cxx::FunctionDeclaration>(
          qname,
          visitor.signature(decl->getReturnType(), decl->parameters()),
          maybe(method),
          range
        );

        if (decl->hasAttrs()) {
          for (const auto attr : decl->getAttrs()) {
            visitor.db.fact<Cxx::FunctionAttribute>(
              visitor.db.fact<Cxx::Attribute>(visitor.db.srcText(attr->getRange()).str()),
              decl_fact
            );
          }
        }
        return FunDecl{
            {},
            qname,
            decl_fact,
            method.has_value()};
      } else {
        return folly::none;
      }
    }

    void define(ASTVisitor& visitor, const clang::FunctionDecl *d) const {
      visitor.db.fact<Cxx::FunctionDefinition>(
        decl,
        d->isInlineSpecified());

      if (method) {
        if (auto mtd = clang::dyn_cast<clang::CXXMethodDecl>(d)) {
          for (const auto *base : mtd->overridden_methods()) {
            if (auto cbase = visitor.funDecls(base)) {
              visitor.db.fact<Cxx::MethodOverrides>(decl, cbase->decl);
            }
          }
        }
      }
    }

    static const clang::FunctionDecl * FOLLY_NULLABLE getInstantiatedMember(
        const clang::FunctionDecl *decl) {
      return decl->getInstantiatedFromMemberFunction();
    }

    static const clang::FunctionDecl * FOLLY_NULLABLE getSpecializedDecl(
        const clang::FunctionDecl *decl) {
      if (auto spec = decl->getTemplateSpecializationInfo()) {
        return spec->getTemplate()->getTemplatedDecl();
      } else {
        return nullptr;
      }
    }
  };

  bool VisitFunctionDecl(const clang::FunctionDecl *decl) {
    visitDeclaration(funDecls, decl);
    return true;
  }

  /*************
   * Variables *
   *************/

  struct VarDecl : Declare<VarDecl> {
    Fact<Cxx::QName> qname;
    Fact<Cxx::VariableDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::variable(decl);
    }

    static folly::Optional<Cxx::GlobalVariableKind> globalKind(
        const clang::VarDecl *decl) {
      if (decl->isLocalVarDeclOrParm()) {
        return folly::none;
      }
      if (decl->isStaticDataMember()) {
        return Cxx::GlobalVariableKind::StaticMember;
      } else {
        switch (decl->getStorageClass()) {
          case clang::SC_None: return Cxx::GlobalVariableKind::SimpleVariable;
          case clang::SC_Extern: return Cxx::GlobalVariableKind::SimpleVariable;
          case clang::SC_Static: return Cxx::GlobalVariableKind::StaticVariable;
          case clang::SC_PrivateExtern: return folly::none;
          case clang::SC_Auto: return folly::none;
          case clang::SC_Register: return folly::none;
        }
      }
    }

    static Cxx::GlobalVariableAttribute globalAttribute(
        const clang::VarDecl *decl) {
      if (decl->isConstexpr()) {
        return Cxx::GlobalVariableAttribute::Constexpr;
      } else if (decl->isInline()) {
        return Cxx::GlobalVariableAttribute::Inline;
      } else {
        return Cxx::GlobalVariableAttribute::Plain;
      }
    }

    static folly::Optional<VarDecl> declare(
        ASTVisitor& visitor,
        const clang::VarDecl *decl,
        Cxx::Scope scope,
        Src::Range range) {
      if (auto kind = globalKind(decl)) {
        auto qname = visitor.db.fact<Cxx::QName>(
          visitor.db.name(decl->getName()), scope);

        return VarDecl
            { {}
            , qname
            , visitor.db.fact<Cxx::VariableDeclaration>(
                qname,
                visitor.type(decl->getType()),
                Cxx::VariableKind::global_(
                  Cxx::GlobalVariable{
                    kind.value(),
                    globalAttribute(decl),
                    decl->isThisDeclarationADefinition()
                      == clang::VarDecl::Definition
                }),
                range)
            };
      } else {
        return folly::none;
      }
    }

    static Cxx::VariableKind fieldKind(
        ASTVisitor& visitor,
        const clang::FieldDecl *decl) {
      folly::Optional<uint64_t> bitsize;
      if (auto size_expr = decl->getBitWidth()) {
        // Consider the following code:
        //
        // template<class T> class U { unsigned i : sizeof(T); };
        //
        // Here, i is a bit field but it doesn't have a fixed bit size. In fact,
        // Clang segfaults if we call getBitWidthValue on it. So let's give it
        // size 0 for now - we should probably extend the schema eventually.
        bitsize = size_expr->isValueDependent()
          ? 0
          : decl->getBitWidthValue(visitor.astContext);
      }
      if (auto ivar = clang::dyn_cast<clang::ObjCIvarDecl>(decl)) {
        return Cxx::VariableKind::ivar(
          Cxx::ObjcIVar{ivar->getSynthesize(), maybe(bitsize)}
        );
      } else {
        return Cxx::VariableKind::field(
          Cxx::Field{decl->isMutable(), maybe(bitsize)}
        );
      }
    }

    static folly::Optional<VarDecl> declare(
        ASTVisitor& visitor,
        const clang::FieldDecl *decl,
        Cxx::Scope scope,
        Src::Range range) {
      auto qname = visitor.db.fact<Cxx::QName>(
        visitor.db.name(decl->getName()), scope);

      return VarDecl
        { {}
        , qname
        , visitor.db.fact<Cxx::VariableDeclaration>(
            qname,
            visitor.type(decl->getType()),
            fieldKind(visitor, decl),
            range)
        };
    }

    void define(ASTVisitor&, const clang::VarDecl *) const {}
    void define(ASTVisitor&, const clang::FieldDecl *) const {}

    static const clang::VarDecl * FOLLY_NULLABLE getInstantiatedMember(
        const clang::VarDecl *decl) {
      return decl->getInstantiatedFromStaticDataMember();
    }

    static const clang::VarDecl * FOLLY_NULLABLE getSpecializedDecl(
        const clang::VarDecl *decl) {
      if (auto spec =
          clang::dyn_cast<clang::VarTemplateSpecializationDecl>(decl)) {
        auto tpl = spec->getSpecializedTemplateOrPartial();
        if (auto tdecl = tpl.dyn_cast<clang::VarTemplateDecl*>()) {
          return tdecl->getTemplatedDecl();
        } else if (auto tspec =
            tpl.dyn_cast<clang::VarTemplatePartialSpecializationDecl*>()) {
          return tspec;
        }
      }
      return nullptr;
    }
  };

  bool VisitVarDecl(const clang::VarDecl *decl) {
    visitDeclaration(varDecls, decl);
    return true;
  }

  bool VisitFieldDecl(const clang::FieldDecl *decl) {
    visitDeclaration(varDecls, decl);
    return true;
  }


  /*******************
   * ObjC containers *
   *******************/

  std::vector<Fact<Cxx::ObjcContainerDeclaration>> objcContainerProtocols(
      const clang::ObjCContainerDecl *decl) {
    const clang::ObjCProtocolList *list;
    if (auto prot = clang::dyn_cast<clang::ObjCProtocolDecl>(decl)) {
      list = &prot->getReferencedProtocols();
    } else if (auto iface = clang::dyn_cast<clang::ObjCInterfaceDecl>(decl)) {
      list = &iface->getReferencedProtocols();
    } else if (auto cat = clang::dyn_cast<clang::ObjCCategoryDecl>(decl)) {
      list = &cat->getReferencedProtocols();
    } else {
      list = nullptr;
    }

    std::vector<Fact<Cxx::ObjcContainerDeclaration>> protocols;
    if (list) {
      if (auto loc = list->loc_begin()) {
        for (auto prot : *list) {
          // NOTE: Protocols don't seem to be visited anywhere so record xrefs
          // here.
          xrefObjCProtocolDecl(*loc, prot);
          ++loc;
          if (auto p = objcContainerDecls(prot)) {
            protocols.push_back(p->decl);
          }
        }
      }
    }

    return protocols;
  }

  Fact<Cxx::Declarations> objcContainerMembers(
      const clang::ObjCContainerDecl *decl) {
    std::vector<Cxx::Declaration> members;

    for (auto member : decl->decls()) {
      if (auto method = clang::dyn_cast<clang::ObjCMethodDecl>(member)) {
        if (auto d = objcMethodDecls(method)) {
          members.push_back(Cxx::Declaration::objcMethod(d->decl));
        }
      }
      if (auto property = clang::dyn_cast<clang::ObjCPropertyDecl>(member)) {
        if (auto d = objcPropertyDecls(property)) {
          members.push_back(Cxx::Declaration::objcProperty(d->decl));
        }
      }
      if (auto ivar = clang::dyn_cast<clang::ObjCIvarDecl>(member)) {
        if (auto d = varDecls(ivar)) {
          members.push_back(Cxx::Declaration::variable(d->decl));
        }
      }
    }
    return db.fact<Cxx::Declarations>(std::move(members));
    }

  struct ObjcContainerDecl : Declare<ObjcContainerDecl> {
    Cxx::ObjcContainerId id;
    Fact<Cxx::ObjcContainerDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::objcContainer(decl);
    }

    template<typename Decl, typename... Decls, typename ClangDecl>
    static folly::Optional<ObjcContainerDecl> findIn(
        ASTVisitor& visitor,
        const ClangDecl *decl) {
      if (auto p = clang::dyn_cast<Decl>(decl)) {
        return visitor.objcContainerDecls(p);
      } else {
        return findIn<Decls...>(visitor, decl);
      }
    }

    template<typename ClangDecl>
    static folly::Optional<ObjcContainerDecl> findIn(
        ASTVisitor&,
        const ClangDecl *) {
      return folly::none;
    }

    template<typename ClangDecl>
    static folly::Optional<ObjcContainerDecl> find(
        ASTVisitor& visitor,
        const ClangDecl * FOLLY_NULLABLE decl) {
      if (decl) {
        return findIn<
                clang::ObjCProtocolDecl,
                clang::ObjCInterfaceDecl,
                clang::ObjCCategoryDecl,
                clang::ObjCImplementationDecl,
                clang::ObjCCategoryImplDecl>(visitor, decl);
        } else {
          return folly::none;
        }
    }

    template<typename Decl>
    static folly::Optional<Cxx::ObjcCategoryId> categoryId(
        ASTVisitor& visitor,
        const Decl *decl) {
      if (auto iface = decl->getClassInterface()) {
        return Cxx::ObjcCategoryId{
          visitor.db.name(iface->getName()),
          visitor.db.name(decl->getName())
        };
      } else {
        return folly::none;
      }
    }

    static folly::Optional<Cxx::ObjcContainerId> containerId(
        ASTVisitor& visitor,
        const clang::ObjCProtocolDecl *decl) {
      return Cxx::ObjcContainerId::protocol(visitor.db.name(decl->getName()));
    }

    static folly::Optional<Cxx::ObjcContainerId> containerId(
        ASTVisitor& visitor,
        const clang::ObjCInterfaceDecl *decl) {
      return Cxx::ObjcContainerId::interface_(
        visitor.db.name(decl->getName()));
    }

    static folly::Optional<Cxx::ObjcContainerId> containerId(
        ASTVisitor& visitor,
        const clang::ObjCCategoryDecl *decl) {
      if (decl->IsClassExtension()) {
        if (auto iface = decl->getClassInterface()) {
          return Cxx::ObjcContainerId::extensionInterface(
            visitor.db.name(iface->getName()));
        } else {
          return folly::none;
        }
      } else if (auto id = categoryId(visitor, decl)) {
        return Cxx::ObjcContainerId::categoryInterface(id.value());
      } else {
        return folly::none;
      }
    }

    static folly::Optional<Cxx::ObjcContainerId> containerId(
        ASTVisitor& visitor,
        const clang::ObjCImplementationDecl *decl) {
      if (auto iface = decl->getClassInterface()) {
        return Cxx::ObjcContainerId::implementation(
          visitor.db.name(iface->getName()));
      } else {
        return folly::none;
      }
    }

    static folly::Optional<Cxx::ObjcContainerId> containerId(
        ASTVisitor& visitor,
        const clang::ObjCCategoryImplDecl *decl) {
      if (auto id = categoryId(visitor, decl)) {
        return Cxx::ObjcContainerId::categoryImplementation(id.value());
      } else {
        return folly::none;
      }
    }

    template<typename Decl>
    static folly::Optional<ObjcContainerDecl> declare(
        ASTVisitor& visitor,
        const Decl *decl,
        Cxx::Scope,
        Src::Range range) {
      if (folly::Optional<Cxx::ObjcContainerId> id
            = ObjcContainerDecl::containerId(visitor, decl)) {
        return ObjcContainerDecl
          { {}
          , id.value()
          , visitor.db.fact<Cxx::ObjcContainerDeclaration>(id.value(), range)
          };
      } else {
        return folly::none;
      }
    }

    static std::vector<ObjcContainerDecl> implements(
        ASTVisitor&,
        const clang::ObjCProtocolDecl *) {
      return {};
    }

    static std::vector<ObjcContainerDecl> implements(
        ASTVisitor&,
        const clang::ObjCInterfaceDecl *) {
      return {};
    }

    static std::vector<ObjcContainerDecl> implements(
        ASTVisitor&,
        const clang::ObjCCategoryDecl *) {
      return {};
    }

    static std::vector<ObjcContainerDecl> implements(
        ASTVisitor& visitor,
        const clang::ObjCImplementationDecl *decl) {
      std::vector<ObjcContainerDecl> decls;
      if (auto iface = decl->getClassInterface()) {
        if (auto r = visitor.objcContainerDecls(iface)) {
          decls.push_back(r.value());
        }
        for (auto cat : iface->known_extensions()) {
          if (auto r = visitor.objcContainerDecls(cat)) {
            decls.push_back(r.value());
          }
        }
      }
      return decls;
    }

    static std::vector<ObjcContainerDecl> implements(
        ASTVisitor& visitor,
        const clang::ObjCCategoryImplDecl *decl) {
      std::vector<ObjcContainerDecl> decls;
        if (auto cat = decl->getCategoryDecl()) {
        if (auto r = visitor.objcContainerDecls(cat)) {
          decls.push_back(r.value());
        }
      }
      return decls;
    }

    static folly::Optional<ObjcContainerDecl> base(
        ASTVisitor&,
        const clang::ObjCImplementationDecl *) {
      return folly::none;
    }

    static folly::Optional<ObjcContainerDecl> base(
        ASTVisitor&,
        const clang::ObjCCategoryDecl*) {
      return folly::none;
    }

    static folly::Optional<ObjcContainerDecl> base(
        ASTVisitor&,
        const clang::ObjCProtocolDecl*) {
      return folly::none;
    }

    static folly::Optional<ObjcContainerDecl> base(
        ASTVisitor& visitor,
        const clang::ObjCInterfaceDecl* decl) {
      if (auto sclass = decl->getSuperClass()) {
        if (auto r = visitor.objcContainerDecls(sclass)) {
          return r.value();
        }
      }
      return folly::none;
    }

    static folly::Optional<ObjcContainerDecl> base(
        ASTVisitor&,
        const clang::ObjCCategoryImplDecl*) {
      return folly::none;
    }

    template<typename Decl>
    void define(ASTVisitor& visitor, const Decl *d) {
      visitor.db.fact<Cxx::ObjcContainerDefinition>(
        decl,
        visitor.objcContainerProtocols(d),
        visitor.objcContainerMembers(d));

      auto xs = implements(visitor, d);
      for (auto x : xs) {
        visitor.db.fact<Cxx::ObjcImplements>(decl, x.decl);
      }

      if(auto bs = base(visitor, d)) {
        visitor.db.fact<Cxx::ObjcContainerBase>(decl, bs->decl);
      }
    }
  };

  bool VisitObjCProtocolDecl(const clang::ObjCProtocolDecl *decl) {
    visitDeclaration(objcContainerDecls, decl);
    return true;
  }

  bool VisitObjCInterfaceDecl(const clang::ObjCInterfaceDecl *decl) {
    visitDeclaration(objcContainerDecls, decl);
    return true;
  }

  bool VisitObjCCategoryDecl(const clang::ObjCCategoryDecl *decl) {
    visitDeclaration(objcContainerDecls, decl);
    return true;
  }

  bool VisitObjCImplementationDecl(const clang::ObjCImplementationDecl *decl) {
    visitDeclaration(objcContainerDecls, decl);
    return true;
  }

  bool VisitObjCCategoryImplDecl(const clang::ObjCCategoryImplDecl *decl) {
    visitDeclaration(objcContainerDecls, decl);
    return true;
  }

  /****************
   * ObjC methods *
   ****************/

  Fact<Cxx::ObjcSelector> objcSelector(const clang::Selector& sel) {
    std::vector<std::string> sels;
    if (sel.isUnarySelector()) {
      // It seems that for unary selectors (i.e., selectors which are just a
      // name with no arguments), getNameForSlot(0) returns "".
      sels.push_back(sel.getAsString());
    } else {
      const size_t n = sel.getNumArgs();
      for (size_t i = 0; i < n; ++i) {
        sels.push_back(static_cast<std::string>(sel.getNameForSlot(i)));
      }
    }
    return db.fact<Cxx::ObjcSelector>(sels);
  }

  struct ObjcMethodDecl : Declare<ObjcMethodDecl> {
    Fact<Cxx::ObjcMethodDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::objcMethod(decl);
    }

    static folly::Optional<ObjcMethodDecl> declare(
        ASTVisitor& visitor,
        const clang::ObjCMethodDecl *d,
        Cxx::Scope,
        Src::Range range) {
      if (auto container =
            ObjcContainerDecl::find(visitor, d->getDeclContext())) {
        return ObjcMethodDecl
          { {}
          , visitor.db.fact<Cxx::ObjcMethodDeclaration>(
              visitor.objcSelector(d->getSelector()),
              container->id,
              visitor.signature(d->getReturnType(), d->parameters()),
              d->isInstanceMethod(),
              d->isOptional(),
              d->isPropertyAccessor(),
              range)
          };
      } else {
        return folly::none;
      }
    }

    void define(ASTVisitor& visitor, const clang::ObjCMethodDecl *) const {
      visitor.db.fact<Cxx::ObjcMethodDefinition>(decl);
    }
  };

  bool VisitObjCMethodDecl(const clang::ObjCMethodDecl *decl) {
    visitDeclaration(objcMethodDecls, decl);
    return true;
  }

  /*******************
   * ObjC properties *
   *******************/

  struct ObjcPropertyDecl : Declare<ObjcPropertyDecl> {
    Fact<Cxx::ObjcPropertyDeclaration> decl;

    Cxx::Declaration declaration() const {
      return Cxx::Declaration::objcProperty(decl);
    }

    static folly::Optional<ObjcPropertyDecl> declare(
        ASTVisitor& visitor,
        const clang::ObjCPropertyDecl *d,
        Cxx::Scope,
        Src::Range range) {
      if (auto container =
            ObjcContainerDecl::find(visitor, d->getDeclContext())) {
        return ObjcPropertyDecl
          { {}
          , visitor.db.fact<Cxx::ObjcPropertyDeclaration>(
              visitor.db.name(d->getName()),
              container->id,
              visitor.type(d->getType()),
              d->isInstanceProperty(),
              d->isOptional(),
              d->isReadOnly(),
              d->isAtomic(),
              range)
          };
      } else {
        return folly::none;
      }
    }

    void define(ASTVisitor&, const clang::ObjCPropertyDecl *) const {
      // TODO: complete
    }
  };

  bool VisitObjCPropertyDecl(const clang::ObjCPropertyDecl *decl) {
    visitDeclaration(objcPropertyDecls, decl);
    return true;
  }

  bool VisitObjCPropertyImplDecl(const clang::ObjCPropertyImplDecl *decl) {
    if (decl->getSourceRange().isValid()) {
      if (auto r = objcPropertyDecls(decl->getPropertyDecl())) {
        // TODO: remove?
        folly::Optional<Fact<Cxx::Name>> name;
        if (decl->isIvarNameSpecified()) {
          name = db.name(decl->getPropertyIvarDecl()->getName());
        }

        if (auto ivar = decl->getPropertyIvarDecl()) {
          if (auto s = varDecls(ivar)) {
            db.fact<Cxx::ObjcPropertyIVar>(r->decl, s->decl);
          }
        }

        db.fact<Cxx::ObjcPropertyImplementation>(
          r->decl,
          decl->getPropertyImplementation()
            == clang::ObjCPropertyImplDecl::Synthesize
            ? Cxx::ObjcPropertyKind::Synthesize
            : Cxx::ObjcPropertyKind::Dynamic,
          maybe(name),
          db.srcRange(decl->getSourceRange()).range);
      }
    }
    return true;
  }

  struct XRef {
    const clang::Decl *primary;
    const clang::Decl *decl;
    folly::Optional<Cxx::XRefTarget> target;

    static XRef unknown(const clang::Decl *d) {
      return XRef{d,d,folly::none};
    }

    static XRef to(const clang::Decl *d, folly::Optional<Cxx::XRefTarget> t) {
      return XRef{d,d,t};
    }

    template<typename Memo, typename Decl>
    static XRef toDecl(Memo& memo, const Decl *decl) {
      XRef xref{decl, decl};
      if (auto b = memo(decl)) {
        xref.target = Cxx::XRefTarget::declaration(b->declaration());
      }
      return xref;
    }

    template<typename Memo, typename Decl>
    void suggest(Memo& memo, const Decl *d) {
      if (!target) {
        if (auto b = memo(d)) {
          target = Cxx::XRefTarget::declaration(b->declaration());
          decl = d;
        }
      }
    }

    template<typename Memo, typename Decl>
    static XRef toTemplatableDecl(Memo& memo, const Decl *decl) {
      XRef xref = toDecl(memo, decl);
      if (auto mem = Memo::value_type::getInstantiatedMember(decl)) {
        decl = mem;
        xref.suggest(memo, decl);
      } else {
        while (auto tpl = Memo::value_type::getSpecializedDecl(decl)) {
          decl = tpl;
          xref.suggest(memo, decl);
        }
      }
      xref.primary = decl;
      return xref;
    }

  };

  void xrefTarget(clang::SourceRange range, XRef xref) {
    const auto raw = xref.target
      ? xref.target.value()
      : unknownTarget(xref.decl);
    const auto wrapped = usingTracker.retarget(xref.primary, raw);
    folly::Optional<clang::SourceLocation> loc;
    if (raw == wrapped) {
      loc = xref.decl->getBeginLoc();
    }
    db.xref(range, loc, wrapped);
  }

  Cxx::XRefTarget unknownTarget(const clang::Decl* decl) {
    return Cxx::XRefTarget::unknown(db.srcLoc(decl->getBeginLoc()));
  }

  void xrefObjCProtocolDecl(
      clang::SourceLocation loc,
      const clang::ObjCProtocolDecl *decl) {
    if (loc.isValid()) {
      xrefTarget(
        {loc, loc.getLocWithOffset(decl->getIdentifier()->getLength()-1)},
        XRef::toDecl(objcContainerDecls, decl));
    }
  }

  /*****************
   * Type visitors *
   *****************/

  bool VisitTagTypeLoc(clang::TagTypeLoc tloc) {
    if (auto decl = tloc.getDecl()) {
      const auto range = tloc.getSourceRange();
      XRef xref;
      if (auto record = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
        xref = XRef::toTemplatableDecl(classDecls, record);
      } else if (auto enm = clang::dyn_cast<clang::EnumDecl>(decl)) {
        xref = XRef::toDecl(enumDecls, enm);
      } else {
        xref = XRef::unknown(decl);
      }
      xrefTarget(range, xref);
    }
    return true;
  }

  bool VisitTypedefTypeLoc(clang::TypedefTypeLoc tloc) {
    if (auto decl = tloc.getTypedefNameDecl()) {
      xrefTarget(
        tloc.getSourceRange(),
        XRef::toDecl(typeAliasDecls, decl));
    }
    return true;
  }

  void xrefTemplateName(clang::SourceLocation loc, clang::TemplateName name) {
    if (auto decl = name.getAsTemplateDecl()) {
      XRef xref;
      if (auto cls = clang::dyn_cast<clang::ClassTemplateDecl>(decl)) {
        xref = XRef::toTemplatableDecl(classDecls, cls->getTemplatedDecl());
      } else if (auto alias = clang::dyn_cast<clang::TypeAliasTemplateDecl>(
          decl)) {
        xref = XRef::toDecl(typeAliasDecls, alias->getTemplatedDecl());
      } else {
        // We don't want to xref template template parameters and I assume we
        // can't get functions or variables here.
        return;
      }
      xrefTarget(db.rangeOfToken(loc), xref);
    }
  }

  bool VisitTemplateSpecializationTypeLoc(
      clang::TemplateSpecializationTypeLoc tloc) {
    xrefTemplateName(
      tloc.getTemplateNameLoc(),
      tloc.getTypePtr()->getTemplateName());
    return true;
  }

  bool VisitObjCObjectTypeLoc(clang::ObjCObjectTypeLoc tloc) {
    const auto n = tloc.getNumProtocols();
    const auto locs = tloc.getProtocolLocs();
    for (size_t i = 0; i < n; ++i) {
      xrefObjCProtocolDecl(locs[i], tloc.getProtocol(i));
    }
    return true;
  }

  bool VisitObjCInterfaceTypeLoc(clang::ObjCInterfaceTypeLoc tloc) {
    xrefTarget(
      tloc.getLocalSourceRange(),
      XRef::toDecl(objcContainerDecls, tloc.getIFaceDecl()));
    return true;
  }

  bool TraverseElaboratedTypeLoc(clang::ElaboratedTypeLoc tloc) {
    return usingTracker.inNameContext(tloc.getTypePtr()->getQualifier(),
      [&] { return Base::TraverseElaboratedTypeLoc(tloc); });
  }

  bool TraverseTemplateArgumentLoc(const clang::TemplateArgumentLoc& arg) {
    // X::T<U> is an ElaboratedType and so we traverse the T<U> bit in the
    // context of X. This is correct for T but wrong for U so we have to reset
    // the context when traversing template arguments.
    return usingTracker.inNameContext(nullptr, [&] {
      switch (arg.getArgument().getKind()) {
        case clang::TemplateArgument::Template:
        case clang::TemplateArgument::TemplateExpansion:
          // This handles cases like:
          //
          // template<typename T> A {};
          // template<template<typename> typename T> B {};
          //
          // B<A> x;
          //
          // The template argument A isn't a proper type here and hence isn't
          // traversed by the usual machinery as such. It is a TemplateName but
          // those don't have source locations so we have to do it in the parent
          // nodes.
          usingTracker.inNameContext(
            arg.getTemplateQualifierLoc().getNestedNameSpecifier(),
            [&] { xrefTemplateName(
              arg.getTemplateNameLoc(),
              arg.getArgument().getAsTemplateOrTemplatePattern()); });
          break;

        default:
          break;
      }
      return Base::TraverseTemplateArgumentLoc(arg); });
  }

  // TODO: It's not clear if/when this is used instead of
  // TraverseTemplateArgumentLoc
  bool TraverseTemplateArguments(
      const clang::TemplateArgument *args, unsigned num) {
    return usingTracker.inNameContext(nullptr,
      [&] { return Base::TraverseTemplateArguments(args, num); });
  }

  bool TraverseDecl(clang::Decl *decl) {
    clang::DeclContext *context = nullptr;
    if (decl) {
      if (auto tunit = clang::dyn_cast<clang::TranslationUnitDecl>(decl)) {
        context = tunit;
      } else if (auto ns = clang::dyn_cast<clang::NamespaceDecl>(decl)) {
        // FIXME: This isn't quite right, the qualified name should be
        // traversed in the enclosing context.
        context = ns;
      } else if (auto tag = clang::dyn_cast<clang::TagDecl>(decl)) {
        // FIXME: This isn't quite right, the qualified name should be
        // traversed in the enclosing context. Not sure about base classes.
        context = tag;
      } else if (auto fun = clang::dyn_cast<clang::FunctionDecl>(decl)) {
        return usingTracker.inFunction(
          fun,
          [&] { return Base::TraverseDecl(decl); });
      }
    }
    return usingTracker.inContext(
      context,
      [&] { return Base::TraverseDecl(decl); });
  }

  bool TraverseCompoundStmt(clang::CompoundStmt *stmt) {
    return usingTracker.maybeBody(
      stmt,
      [&] { return Base::TraverseCompoundStmt(stmt) ; });
  }

  bool TraverseCoroutineBodyStmt(clang::CoroutineBodyStmt *stmt) {
    return usingTracker.maybeBody(
      stmt,
      [&] { return Base::TraverseCoroutineBodyStmt(stmt) ; });
  }

  bool TraverseLambdaExpr(clang::LambdaExpr *lambda) {
    return usingTracker.inFunction(
      lambda->getCallOperator(),
      [&] { return Base::TraverseLambdaExpr(lambda); });
  }

  bool TraverseParmVarDecl(clang::ParmVarDecl *decl) {
    // Thankfully, ParmVarDecls seem to have the right context
    if (auto *context = decl->getDeclContext()) {
      if (clang::isa<clang::FunctionDecl>(context)) {
        return usingTracker.inContext(
          context,
          [&] { return Base::TraverseParmVarDecl(decl); });
      }
    }
    return Base::TraverseParmVarDecl(decl);
  }

  // TODO: set the right context for constructor initialisers and exception
  // specs


  // NOTE: For some reason, RecursiveASTVisitor doesn't seem to call
  // VisitNestedNameSpecifierLoc
  bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc loc) {
    auto spec = loc.getNestedNameSpecifier();
    return usingTracker.inNameContext(spec ? spec->getPrefix() : nullptr,
      [&] {
        if (spec) {
          if (auto ns = spec->getAsNamespace()) {
            xrefTarget(
              db.rangeOfToken(loc.getLocalSourceRange()),
              XRef::toDecl(namespaces, ns));
          }
        }
        return Base::TraverseNestedNameSpecifierLoc(loc);
      });
  }

  void xrefExpr(
      const clang::Decl * FOLLY_NULLABLE decl,
      const clang::NestedNameSpecifier * FOLLY_NULLABLE qualifier,
      clang::SourceRange range) {
    if (decl) {
      // TODO: We aren't cross-referencing local variables for now but should
      // eventually.
      if (!clang::isa<clang::VarDecl>(decl)
            || decl->getParentFunctionOrMethod() == nullptr) {
        auto xref = XRef::unknown(decl);
        if (auto fun = clang::dyn_cast<clang::FunctionDecl>(decl)) {
          xref = XRef::toTemplatableDecl(funDecls, fun);
        } else if (auto var = clang::dyn_cast<clang::VarDecl>(decl)) {
          xref = XRef::toTemplatableDecl(varDecls, var);
        } else if (auto field = clang::dyn_cast<clang::FieldDecl>(decl)) {
          // TODO: can this ever happen? or will it always be a MemberExpr?
          xref = XRef::toDecl(varDecls, field);
        } else if (auto e = clang::dyn_cast<clang::EnumConstantDecl>(decl)) {
          if (auto r = enumeratorDecls(e)) {
            xref = XRef::to(e, Cxx::XRefTarget::enumerator(r->fact));
          }
        }
        usingTracker.inNameContext(qualifier, [&]{ xrefTarget(range, xref); });
      }
    }
  }

  // When using macros, the SourceLocation refers to the post expansion location.
  // This is not useful to users who are looking at the pre-expansion location in
  // their code.
  clang::SourceLocation fixMacroLocation(clang::SourceLocation loc) {
    const auto& srcMgr = db.sourceManager();
    if (!srcMgr.isMacroArgExpansion(loc)) {
      return srcMgr.getExpansionLoc(loc);
    }
    // If a macro arg is a result of a paste, its spelling is in scratch space.
    //
    // #define PASTE(x,y) x##y
    // #define MACRO(x) x
    //
    // MACRO(PASTE(foo,bar))
    //
    // If this is the case, we walk up the parent context
    // since there can be several level of token pasting.
    if (srcMgr.isWrittenInScratchSpace(srcMgr.getSpellingLoc(loc))) {
      do {
        loc = srcMgr.getImmediateMacroCallerLoc(loc);
      } while (srcMgr.isWrittenInScratchSpace(srcMgr.getSpellingLoc(loc)));
      return srcMgr.getExpansionLoc(loc);
    }
    return srcMgr.getSpellingLoc(loc);
  }

  bool VisitDeclRefExpr(const clang::DeclRefExpr* expr) {
    auto const beginLoc = expr->getNameInfo().getSourceRange().getBegin();
    auto const endLoc = expr->getNameInfo().getSourceRange().getEnd();
    xrefExpr(
      expr->getDecl(),
      expr->getQualifier(),
      { fixMacroLocation(beginLoc), fixMacroLocation(endLoc)});
    return true;
  }

  bool VisitOverloadExpr(const clang::OverloadExpr *expr) {
    const auto qualifier = expr->getQualifier();
    const auto range = expr->getNameInfo().getSourceRange();
    // For now, we xref all functions in the overload set:
    //
    // void f(int); // A
    // void f(bool); // B
    //
    // template<class T> void g(T x) { f(x); } // will xref both A and B
    //
    // This is especially important for using decls.
    for (const auto *decl : expr->decls()) {
      if (auto shadow = clang::dyn_cast_or_null<clang::UsingShadowDecl>(decl)) {
        decl = shadow->getTargetDecl();
      }
      if (auto tpl = clang::dyn_cast_or_null<clang::TemplateDecl>(decl)) {
        decl = tpl->getTemplatedDecl();
      }
      xrefExpr(decl, qualifier, range);
    }
    return true;
  }

  bool VisitMemberExpr(const clang::MemberExpr *expr) {
    if (const auto *decl = expr->getMemberDecl()) {
      XRef xref;
      folly::Optional<Cxx::XRefTarget> target;
      if (auto fun = clang::dyn_cast<clang::FunctionDecl>(decl)) {
        xref = XRef::toTemplatableDecl(funDecls, fun);
      } else if (auto field = clang::dyn_cast<clang::FieldDecl>(decl)) {
        xref = XRef::toDecl(varDecls, field);
      } else {
        xref = XRef::unknown(decl);
      }

      // getMemberLoc can return an invalid location if, say, an implicit
      // conversion operator is applied to the member. This seems to be a bug
      // either in Clang proper or at least in the docs.
      auto begin = expr->getMemberLoc();
      if (!begin.isValid()) {
        begin = expr->getBeginLoc();
      }

      xrefTarget(clang::SourceRange(begin, expr->getEndLoc()), xref);
    }
    return true;
  }

  bool VisitCXXDependentScopeMemberExpr(
      const clang::CXXDependentScopeMemberExpr *expr) {
    if (const auto *decl = expr->getFirstQualifierFoundInScope()) {
      xrefTarget(
        clang::SourceRange(
          expr->getMemberLoc(),
          expr->
#if LLVM_VERSION_MAJOR >= 8
            getEndLoc()
#else
            getLocEnd()
#endif
        ),
        XRef::unknown(decl));
    }
    return true;
  }

  bool VisitObjCMessageExpr(const clang::ObjCMessageExpr *expr) {
    if (!expr->isImplicit()) {
      if (const auto *decl = expr->getMethodDecl()) {
        folly::Optional<Cxx::XRefTarget> target;
        if (auto r = objcMethodDecls(decl)) {
          target = Cxx::XRefTarget::declaration(
            Cxx::Declaration::objcMethod(r->decl));
        }
        const auto sel = expr->getSelector();
        const auto nsels = expr->getNumSelectorLocs();
        for (unsigned int i = 0; i < nsels; ++i) {
          const auto start = expr->getSelectorLoc(i);
          if (start.isValid()) {
            // NOTE: Arguments don't necessarily have names, e.g.
            // foo:(float)x :(float)y :(float)z
            // which would then be called as
            // [o foo:1 :2 :3]
            //
            // There will be at least one word (the method name) so we'll
            // definitely get at least one xref.
            if (auto ident = sel.getIdentifierInfoForSlot(i)) {
              const auto end = start.getLocWithOffset(
                ident->getLength()-1);
              xrefTarget(
                clang::SourceRange(start,end),
                XRef::to(decl, target));
            }
          }
        }
      }
    }
    return true;
  }

  bool VisitObjCPropertyRefExpr(const clang::ObjCPropertyRefExpr *expr) {
    std::vector<XRef> xrefs;
    if (expr->isImplicitProperty()) {
      // Note things like x.foo += 5 generate xrefs to both getter and setter.
      if (expr->isMessagingGetter()) {
        if (auto getter = expr->getImplicitPropertyGetter()) {
          xrefs.push_back(XRef::toDecl(objcMethodDecls, getter));
        }
        if (expr->isClassReceiver()) {
          xrefTarget(
            expr->getReceiverLocation(),
            XRef::toDecl(objcContainerDecls, expr->getClassReceiver())
          );
        }
      }
      if (expr->isMessagingSetter()) {
        if (auto setter = expr->getImplicitPropertySetter()) {
          xrefs.push_back(XRef::toDecl(objcMethodDecls, setter));
        }
      }
    } else if (auto prop = expr->getExplicitProperty()) {
      xrefs.push_back(XRef::toDecl(objcPropertyDecls, prop));
    }

    if (!xrefs.empty()) {
      const auto start = expr->getLocation();
      const auto range = db.rangeOfToken(start);
      for (const auto &xref : xrefs) {
        xrefTarget(range, xref);
      }
    }
    return true;
  }

  bool VisitObjCIvarRefExpr(const clang::ObjCIvarRefExpr *expr) {
    const clang::ObjCIvarDecl *decl = expr->getDecl();
    xrefTarget(db.rangeOfToken(expr->getLocation()), XRef::toDecl(varDecls, decl));
    return true;
  }

  bool VisitObjCSelectorExpr(const clang::ObjCSelectorExpr *expr) {
    db.xref(
      clang::SourceRange(expr->getBeginLoc(), expr->getEndLoc()),
      folly::none,
      Cxx::XRefTarget::objcSelector(objcSelector(expr->getSelector()))
    );
    return true;
  }

  bool VisitObjCProtocolExpr(const clang::ObjCProtocolExpr *expr) {
    xrefObjCProtocolDecl(expr->getBeginLoc(), expr->getProtocol());
    return true;
  }

  bool shouldVisitTemplateInstantiations() const {
    return false;
  }

  void finish() {
    db.finish();
  }

  ASTVisitor(ClangDB* d, clang::ASTContext& ctx)
    : db(*d)
    , astContext(ctx)
    , usingTracker(ctx.getTranslationUnitDecl(), *d)
    , scopes(*this)
    , namespaces("namespaces", *this)
    , classDecls("classDecls", *this)
    , enumDecls("enumDecls", *this)
    , enumeratorDecls("enumeratorDecls", *this)
    , typeAliasDecls("typeAliasDecls", *this)
    , funDecls("funDecls", *this)
    , varDecls("varDecls", *this)
    , objcContainerDecls("objcContainerDecls", *this)
    , objcMethodDecls("objcMethodDecls", *this)
    , objcPropertyDecls("objcPropertyDecls", *this)
    {}

  ClangDB& db;
  clang::ASTContext &astContext;

  UsingTracker usingTracker;

  Memo<const clang::DeclContext *,
       Scope,
       &ASTVisitor::defineScope>
    scopes;

  MemoOptional<
      const clang::NamespaceDecl *,
      NamespaceDecl>
    namespaces;

  MemoOptional<
      const clang::CXXRecordDecl *,
      ClassDecl>
    classDecls;

  MemoOptional<
      const clang::EnumDecl *,
      EnumDecl>
    enumDecls;

  MemoOptional<
      const clang::EnumConstantDecl *,
      EnumeratorDecl>
    enumeratorDecls;

  MemoOptional<
      const clang::TypedefNameDecl *,
      TypeAliasDecl>
    typeAliasDecls;

  MemoOptional<
      const clang::FunctionDecl *,
      FunDecl>
    funDecls;

  MemoOptional<
      const clang::DeclaratorDecl *,
      VarDecl>
    varDecls;

  MemoOptional<
      const clang::ObjCContainerDecl *,
      ObjcContainerDecl>
    objcContainerDecls;

  MemoOptional<
      const clang::ObjCMethodDecl *,
      ObjcMethodDecl>
    objcMethodDecls;

  MemoOptional<
      const clang::ObjCPropertyDecl *,
      ObjcPropertyDecl>
    objcPropertyDecls;
};

// ASTConsumer uses ASTVisitor to traverse the Clang AST
//
struct ASTConsumer : public clang::ASTConsumer {
  explicit ASTConsumer(ClangDB* d) : db(d) {}

  void HandleTranslationUnit (clang::ASTContext &ctx) override {
    // Clang is sometimes generating a bogus AST when there are
    // compilation errors (even for parts of the AST that should be
    // unrelated to the error) so this is a workaround.
    if (ctx.getDiagnostics().hasErrorOccurred() && !FLAGS_index_on_error) {
      db->IndexFailure(ctx);
      return;
    }
    ASTVisitor visitor(db, ctx);
    VLOG(1) << "traversing";
    visitor.TraverseDecl(ctx.getTranslationUnitDecl());
    visitor.finish();
  }

  ClangDB* db;
};

}

namespace facebook {
namespace glean {
namespace clangx {

std::unique_ptr<clang::ASTConsumer> newASTConsumer(ClangDB* db) {
  return std::make_unique<ASTConsumer>(db);
}

}
}
}