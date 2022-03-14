"use strict";(self.webpackChunkwebsite=self.webpackChunkwebsite||[]).push([[917],{3905:function(e,n,a){a.r(n),a.d(n,{MDXContext:function(){return c},MDXProvider:function(){return p},mdx:function(){return f},useMDXComponents:function(){return m},withMDXComponents:function(){return l}});var t=a(67294);function i(e,n,a){return n in e?Object.defineProperty(e,n,{value:a,enumerable:!0,configurable:!0,writable:!0}):e[n]=a,e}function r(){return r=Object.assign||function(e){for(var n=1;n<arguments.length;n++){var a=arguments[n];for(var t in a)Object.prototype.hasOwnProperty.call(a,t)&&(e[t]=a[t])}return e},r.apply(this,arguments)}function o(e,n){var a=Object.keys(e);if(Object.getOwnPropertySymbols){var t=Object.getOwnPropertySymbols(e);n&&(t=t.filter((function(n){return Object.getOwnPropertyDescriptor(e,n).enumerable}))),a.push.apply(a,t)}return a}function s(e){for(var n=1;n<arguments.length;n++){var a=null!=arguments[n]?arguments[n]:{};n%2?o(Object(a),!0).forEach((function(n){i(e,n,a[n])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(a)):o(Object(a)).forEach((function(n){Object.defineProperty(e,n,Object.getOwnPropertyDescriptor(a,n))}))}return e}function d(e,n){if(null==e)return{};var a,t,i=function(e,n){if(null==e)return{};var a,t,i={},r=Object.keys(e);for(t=0;t<r.length;t++)a=r[t],n.indexOf(a)>=0||(i[a]=e[a]);return i}(e,n);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);for(t=0;t<r.length;t++)a=r[t],n.indexOf(a)>=0||Object.prototype.propertyIsEnumerable.call(e,a)&&(i[a]=e[a])}return i}var c=t.createContext({}),l=function(e){return function(n){var a=m(n.components);return t.createElement(e,r({},n,{components:a}))}},m=function(e){var n=t.useContext(c),a=n;return e&&(a="function"==typeof e?e(n):s(s({},n),e)),a},p=function(e){var n=m(e.components);return t.createElement(c.Provider,{value:n},e.children)},h={inlineCode:"code",wrapper:function(e){var n=e.children;return t.createElement(t.Fragment,{},n)}},u=t.forwardRef((function(e,n){var a=e.components,i=e.mdxType,r=e.originalType,o=e.parentName,c=d(e,["components","mdxType","originalType","parentName"]),l=m(a),p=i,u=l["".concat(o,".").concat(p)]||l[p]||h[p]||r;return a?t.createElement(u,s(s({ref:n},c),{},{components:a})):t.createElement(u,s({ref:n},c))}));function f(e,n){var a=arguments,i=n&&n.mdxType;if("string"==typeof e||i){var r=a.length,o=new Array(r);o[0]=u;var s={};for(var d in n)hasOwnProperty.call(n,d)&&(s[d]=n[d]);s.originalType=e,s.mdxType="string"==typeof e?e:i,o[1]=s;for(var c=2;c<r;c++)o[c]=a[c];return t.createElement.apply(null,o)}return t.createElement.apply(null,a)}u.displayName="MDXCreateElement"},92721:function(e,n,a){a.r(n),a.d(n,{frontMatter:function(){return s},contentTitle:function(){return d},metadata:function(){return c},toc:function(){return l},default:function(){return p}});var t=a(87462),i=a(63366),r=(a(67294),a(3905)),o=["components"],s={id:"changing",title:"How do I change a schema?",sidebar_label:"Changing a schema"},d=void 0,c={unversionedId:"schema/changing",id:"schema/changing",isDocsHomePage:!1,title:"How do I change a schema?",description:"Predicates are never modified. We can only make new versions of a",source:"@site/docs/schema/changing.md",sourceDirName:"schema",slug:"/schema/changing",permalink:"/docs/schema/changing",editUrl:"https://github.com/facebookincubator/Glean/tree/main/glean/website/docs/schema/changing.md",tags:[],version:"current",frontMatter:{id:"changing",title:"How do I change a schema?",sidebar_label:"Changing a schema"},sidebar:"someSidebar",previous:{title:"Recursion",permalink:"/docs/schema/recursion"},next:{title:'The special "all" schema',permalink:"/docs/schema/all"}},l=[{value:"Adding new predicates",id:"adding-new-predicates",children:[],level:3},{value:"Deleting predicates",id:"deleting-predicates",children:[],level:3},{value:"Schema migrations with backward compatible changes",id:"schema-migrations-with-backward-compatible-changes",children:[{value:"Evolving schemas",id:"evolving-schemas",children:[],level:3}],level:2}],m={toc:l};function p(e){var n=e.components,a=(0,i.Z)(e,o);return(0,r.mdx)("wrapper",(0,t.Z)({},m,a,{components:n,mdxType:"MDXLayout"}),(0,r.mdx)("div",{className:"admonition admonition-important alert alert--info"},(0,r.mdx)("div",{parentName:"div",className:"admonition-heading"},(0,r.mdx)("h5",{parentName:"div"},(0,r.mdx)("span",{parentName:"h5",className:"admonition-icon"},(0,r.mdx)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.mdx)("path",{parentName:"svg",fillRule:"evenodd",d:"M7 2.3c3.14 0 5.7 2.56 5.7 5.7s-2.56 5.7-5.7 5.7A5.71 5.71 0 0 1 1.3 8c0-3.14 2.56-5.7 5.7-5.7zM7 1C3.14 1 0 4.14 0 8s3.14 7 7 7 7-3.14 7-7-3.14-7-7-7zm1 3H6v5h2V4zm0 6H6v2h2v-2z"}))),"important")),(0,r.mdx)("div",{parentName:"div",className:"admonition-content"},(0,r.mdx)("p",{parentName:"div"},"Predicates are never modified. We can only make new versions of a\npredicate, or delete an old version of a predicate when we no longer\nneed to read or write data using it."))),(0,r.mdx)("p",null,"A schema is a contract between indexer, client and server about\nthe shape of facts. Schemas are used during the compilation of some clients to\ngenerate code to build queries and decode facts. Because it is not possible to\nupdate all running application clients and issue new databases in one atomic\noperation, if we change the shape of a predicate type clients will suddenly\nbegin to create type-incorrect queries and become unable to decode facts."),(0,r.mdx)("p",null,"Because of that you can only add new predicates or new versions of predicates, and\ndelete old ones. This is to ensure compatibilty between different\nversions of clients and databases: adding new predicates to the schema\ndoesn't break existing clients or indexers."),(0,r.mdx)("p",null,"To be specific, ",(0,r.mdx)("em",{parentName:"p"},"modifying")," a predicate means changing its type in any way. To modify a predicate you need to:"),(0,r.mdx)("ul",null,(0,r.mdx)("li",{parentName:"ul"},"Add a new version of the predicate, creating a new schema version at the same time if necessary.",(0,r.mdx)("ul",{parentName:"li"},(0,r.mdx)("li",{parentName:"ul"},"This may entail adding new versions of other predicates too, because predicates that depended on the old version of the predicate must now be copied so that they can point to the new predicate you created."))),(0,r.mdx)("li",{parentName:"ul"},"Update and recompile clients and indexers as necessary to use your new version. Most of the time we don't use explicit versions in client code, so usually updating a client is just a recompile after the schema update.")),(0,r.mdx)("p",null,"Changing the schema can present a tricky migration problem: there are indexers generating the data, clients reading the data, and existing databases that can contain either the old schema or the new schema. Glean provides features to make smooth migrations possible, see ",(0,r.mdx)("a",{parentName:"p",href:"/docs/derived#derived-predicates-for-schema-migration"},"Derived Predicates for Schema Migration")," and ",(0,r.mdx)("a",{parentName:"p",href:"#schema-migrations-with-backward-compatible-changes"},"Schema migrations with backward compatible changes")),(0,r.mdx)("div",{className:"admonition admonition-note alert alert--secondary"},(0,r.mdx)("div",{parentName:"div",className:"admonition-heading"},(0,r.mdx)("h5",{parentName:"div"},(0,r.mdx)("span",{parentName:"h5",className:"admonition-icon"},(0,r.mdx)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.mdx)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,r.mdx)("div",{parentName:"div",className:"admonition-content"},(0,r.mdx)("p",{parentName:"div"},"if you're just changing the derivation of a derived predicate, there's no need to create a new predicate version. The new derivation will take effect, for both old and new databases, as soon as the schema change is deployed."))),(0,r.mdx)("h3",{id:"adding-new-predicates"},"Adding new predicates"),(0,r.mdx)("p",null,"If you're just adding new predicates or types, then you don't need to create a new schema version."),(0,r.mdx)("h3",{id:"deleting-predicates"},"Deleting predicates"),(0,r.mdx)("p",null,"In most cases it's safe to delete predicates from the schema, provided you have no existing client code using them."),(0,r.mdx)("h2",{id:"schema-migrations-with-backward-compatible-changes"},"Schema migrations with backward compatible changes"),(0,r.mdx)("p",null,"One of the challenges of migrations is that once you start producing databases with a new schema, clients which specify predicate versions in their queries will stop receiving results until they are updated to request the latest available version. If the client is updated first we have a similar problem; it will not receive results until databases with the latest schema are produced."),(0,r.mdx)("p",null,"To allow old clients which specify predicate versions to still receive results when schemas are updated Glean supports ",(0,r.mdx)("strong",{parentName:"p"},"schema evolution"),", a feature where facts of a new schema can be automatically transformed into facts of an older schema to be returned to old clients."),(0,r.mdx)("p",null,"To use schema evolutions, all changes made in the new schema must be backward compatible. The following are the supported backward compatible changes:"),(0,r.mdx)("ul",null,(0,r.mdx)("li",{parentName:"ul"},"Add a field to a predicate/type"),(0,r.mdx)("li",{parentName:"ul"},"Change field order in a predicate/type"),(0,r.mdx)("li",{parentName:"ul"},"Change alternative order in a sum type or enum"),(0,r.mdx)("li",{parentName:"ul"},"Add a predicate"),(0,r.mdx)("li",{parentName:"ul"},"Remove a predicate")),(0,r.mdx)("p",null,"Changes that are not backward compatible are not supported, such as:"),(0,r.mdx)("ul",null,(0,r.mdx)("li",{parentName:"ul"},"Remove a field"),(0,r.mdx)("li",{parentName:"ul"},"Change the type of a field")),(0,r.mdx)("div",{className:"admonition admonition-note alert alert--secondary"},(0,r.mdx)("div",{parentName:"div",className:"admonition-heading"},(0,r.mdx)("h5",{parentName:"div"},(0,r.mdx)("span",{parentName:"h5",className:"admonition-icon"},(0,r.mdx)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"14",height:"16",viewBox:"0 0 14 16"},(0,r.mdx)("path",{parentName:"svg",fillRule:"evenodd",d:"M6.3 5.69a.942.942 0 0 1-.28-.7c0-.28.09-.52.28-.7.19-.18.42-.28.7-.28.28 0 .52.09.7.28.18.19.28.42.28.7 0 .28-.09.52-.28.7a1 1 0 0 1-.7.3c-.28 0-.52-.11-.7-.3zM8 7.99c-.02-.25-.11-.48-.31-.69-.2-.19-.42-.3-.69-.31H6c-.27.02-.48.13-.69.31-.2.2-.3.44-.31.69h1v3c.02.27.11.5.31.69.2.2.42.31.69.31h1c.27 0 .48-.11.69-.31.2-.19.3-.42.31-.69H8V7.98v.01zM7 2.3c-3.14 0-5.7 2.54-5.7 5.68 0 3.14 2.56 5.7 5.7 5.7s5.7-2.55 5.7-5.7c0-3.15-2.56-5.69-5.7-5.69v.01zM7 .98c3.86 0 7 3.14 7 7s-3.14 7-7 7-7-3.12-7-7 3.14-7 7-7z"}))),"note")),(0,r.mdx)("div",{parentName:"div",className:"admonition-content"},(0,r.mdx)("p",{parentName:"div"},"When making a new schema version you may also have to update the\nspecial ",(0,r.mdx)("inlineCode",{parentName:"p"},"all")," schema to ensure a smooth migration for clients that are\nusing unversioned queries. See ",(0,r.mdx)("a",{parentName:"p",href:"../all"},'The special "all" schema'),"."))),(0,r.mdx)("h3",{id:"evolving-schemas"},"Evolving schemas"),(0,r.mdx)("p",null,"The feature is enabled using a top-level directive"),(0,r.mdx)("pre",null,(0,r.mdx)("code",{parentName:"pre"},"schema my_schema.2 evolves my_schema.1\n")),(0,r.mdx)("p",null,"This declaration has the effect of treating queries for ",(0,r.mdx)("inlineCode",{parentName:"p"},"my_schema.1")," predicates as if they were for ",(0,r.mdx)("inlineCode",{parentName:"p"},"my_schema.2"),". That is the query results will be retrieved from the database in the shame of a ",(0,r.mdx)("inlineCode",{parentName:"p"},"my_schema.2")," fact and transformed into a fact of the equivalent ",(0,r.mdx)("inlineCode",{parentName:"p"},"my_schema.1")," predicate specified in the query."),(0,r.mdx)("p",null,"The new schema must contain all the predicates of the old schema, either with new versions or old versions, and their definitions must be backwards compatible. We can achieve this by copying the entire content of the old schema into the new one and modifying it there."),(0,r.mdx)("p",null,"Now what should Glean do when a client asks for a fact from an old schema?"),(0,r.mdx)("ul",null,(0,r.mdx)("li",{parentName:"ul"},"Answer with db facts from the old schema"),(0,r.mdx)("li",{parentName:"ul"},"Answer with db facts from the new schema transformed into the old ones.")),(0,r.mdx)("p",null,"If there are no facts of the old schema in in the database we will take option 2.\nIf the database has any fact at all of the old schema we choose option 1."),(0,r.mdx)("p",null,"That is, schema evolutions only take effect if there are no facts of the old schema in the database; it is ignored otherwise."),(0,r.mdx)("p",null,"As an example suppose we start with the following schemas:"),(0,r.mdx)("pre",null,(0,r.mdx)("code",{parentName:"pre"},'schema src.1 {\n   predicate File {\n     path : string\n   }\n}\n\nschema os.1 {\n  import src.1\n\n  predicate Permissions {\n    file : File,\n    permissions : nat\n  }\n}\n\nschema info.1 {\n  import src.1\n\n  predicate IsTemporary {\n    file : File\n  } F where F = src.File { path = "/tmp".. }\n}\n')),(0,r.mdx)("p",null,"Now we want to make a backward-compatible change to ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.File")," and add an ",(0,r.mdx)("inlineCode",{parentName:"p"},"extension")," field. We could add this to the file:"),(0,r.mdx)("pre",null,(0,r.mdx)("code",{parentName:"pre"},"schema src.2 {\n   predicate File {\n     path : string,\n     extension : string\n   }\n}\n\nschema src.2 evolves src.1\n")),(0,r.mdx)("p",null,"Now if the indexer is still producing only ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.1")," facts, all other predicates will work as before and queries for ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.File.2")," will return no results."),(0,r.mdx)("p",null,"Once the indexer is changed to produce only ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.2")," facts queries like ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.File.1 _")," will be fulfilled using data from the ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.2")," schema, converting the ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.File.2")," results to the shape of ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.File.1")," before returning to the client."),(0,r.mdx)("p",null,"This is also the case in the derivation query of ",(0,r.mdx)("inlineCode",{parentName:"p"},"info.IsTemporary"),". Although ",(0,r.mdx)("inlineCode",{parentName:"p"},"info")," imports ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.1"),", the query will be transformed to use ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.2")," facts."),(0,r.mdx)("p",null,"On the other hand, ",(0,r.mdx)("inlineCode",{parentName:"p"},"os.Permissions")," will be empty. This must be the case because its first field references a ",(0,r.mdx)("inlineCode",{parentName:"p"},"src.File.1")," fact, of which there is none in the database. For this predicate to continue being available we must evolve its schema as well."),(0,r.mdx)("pre",null,(0,r.mdx)("code",{parentName:"pre"},"schema os.2 {             # changed\n  import src.2            # changed\n\n  predicate Permissions {\n    file : File,\n    permissions : nat\n  }\n}\n\nschema os.2 evolves os.1    # changed\n")))}p.isMDXComponent=!0}}]);