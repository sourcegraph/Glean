"use strict";(self.webpackChunkwebsite=self.webpackChunkwebsite||[]).push([[731],{3905:function(e,n,t){t.r(n),t.d(n,{MDXContext:function(){return l},MDXProvider:function(){return f},mdx:function(){return h},useMDXComponents:function(){return d},withMDXComponents:function(){return s}});var r=t(67294);function o(e,n,t){return n in e?Object.defineProperty(e,n,{value:t,enumerable:!0,configurable:!0,writable:!0}):e[n]=t,e}function i(){return i=Object.assign||function(e){for(var n=1;n<arguments.length;n++){var t=arguments[n];for(var r in t)Object.prototype.hasOwnProperty.call(t,r)&&(e[r]=t[r])}return e},i.apply(this,arguments)}function a(e,n){var t=Object.keys(e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);n&&(r=r.filter((function(n){return Object.getOwnPropertyDescriptor(e,n).enumerable}))),t.push.apply(t,r)}return t}function c(e){for(var n=1;n<arguments.length;n++){var t=null!=arguments[n]?arguments[n]:{};n%2?a(Object(t),!0).forEach((function(n){o(e,n,t[n])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(t)):a(Object(t)).forEach((function(n){Object.defineProperty(e,n,Object.getOwnPropertyDescriptor(t,n))}))}return e}function u(e,n){if(null==e)return{};var t,r,o=function(e,n){if(null==e)return{};var t,r,o={},i=Object.keys(e);for(r=0;r<i.length;r++)t=i[r],n.indexOf(t)>=0||(o[t]=e[t]);return o}(e,n);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);for(r=0;r<i.length;r++)t=i[r],n.indexOf(t)>=0||Object.prototype.propertyIsEnumerable.call(e,t)&&(o[t]=e[t])}return o}var l=r.createContext({}),s=function(e){return function(n){var t=d(n.components);return r.createElement(e,i({},n,{components:t}))}},d=function(e){var n=r.useContext(l),t=n;return e&&(t="function"==typeof e?e(n):c(c({},n),e)),t},f=function(e){var n=d(e.components);return r.createElement(l.Provider,{value:n},e.children)},p={inlineCode:"code",wrapper:function(e){var n=e.children;return r.createElement(r.Fragment,{},n)}},g=r.forwardRef((function(e,n){var t=e.components,o=e.mdxType,i=e.originalType,a=e.parentName,l=u(e,["components","mdxType","originalType","parentName"]),s=d(t),f=o,g=s["".concat(a,".").concat(f)]||s[f]||p[f]||i;return t?r.createElement(g,c(c({ref:n},l),{},{components:t})):r.createElement(g,c({ref:n},l))}));function h(e,n){var t=arguments,o=n&&n.mdxType;if("string"==typeof e||o){var i=t.length,a=new Array(i);a[0]=g;var c={};for(var u in n)hasOwnProperty.call(n,u)&&(c[u]=n[u]);c.originalType=e,c.mdxType="string"==typeof e?e:o,a[1]=c;for(var l=2;l<i;l++)a[l]=t[l];return r.createElement.apply(null,a)}return r.createElement.apply(null,t)}g.displayName="MDXCreateElement"},2896:function(e,n,t){t.r(n),t.d(n,{frontMatter:function(){return c},contentTitle:function(){return u},metadata:function(){return l},toc:function(){return s},default:function(){return f}});var r=t(83117),o=t(80102),i=(t(67294),t(3905)),a=(t(44256),["components"]),c={id:"debugging",title:"Debugging",sidebar_label:"Debugging"},u=void 0,l={unversionedId:"angle/debugging",id:"angle/debugging",isDocsHomePage:!1,title:"Debugging",description:"Typically you want to do most of your debugging in the",source:"@site/docs/angle/debugging.md",sourceDirName:"angle",slug:"/angle/debugging",permalink:"/docs/angle/debugging",editUrl:"https://github.com/facebookincubator/Glean/tree/main/glean/website/docs/angle/debugging.md",tags:[],version:"current",frontMatter:{id:"debugging",title:"Debugging",sidebar_label:"Debugging"},sidebar:"someSidebar",previous:{title:"Advanced Query Features",permalink:"/docs/angle/advanced"},next:{title:"Reference",permalink:"/docs/angle/reference"}},s=[{value:"Debugging a slow query",id:"debugging-a-slow-query",children:[],level:2},{value:"Showing the internals",id:"showing-the-internals",children:[],level:2}],d={toc:s};function f(e){var n=e.components,t=(0,o.Z)(e,a);return(0,i.mdx)("wrapper",(0,r.Z)({},d,t,{components:n,mdxType:"MDXLayout"}),(0,i.mdx)("p",null,"Typically you want to do most of your debugging in the\n",(0,i.mdx)("a",{parentName:"p",href:"/docs/shell"},"shell"),", where you can experiment with queries quickly and\neasily."),(0,i.mdx)("p",null,"If you're writing particularly complex queries, then consider using ",(0,i.mdx)("a",{parentName:"p",href:"/docs/derived"},"Derived Predicates")," to structure your query and to allow parts of the query to be re-used. To iterate on derived predicates, see ",(0,i.mdx)("a",{parentName:"p",href:"/docs/derived#how-do-i-write-and-test"},"How do I write and test a derived predicate?")),(0,i.mdx)("h2",{id:"debugging-a-slow-query"},"Debugging a slow query"),(0,i.mdx)("p",null,"Performance debugging can be tricky, because Angle is a very declarative language. There are often many ways to write the query that are correct, but not all of them will be fast."),(0,i.mdx)("p",null,"The shell provides a few facilities to help with this."),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},"> :profile full\n")),(0,i.mdx)("p",null,"Turning on query profiling allows you to see how many facts of each predicate are being searched by your query. For example:"),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},'fbsource> search.cxx.SearchByNameAndScope { name = "Future" }\n...\nFacts searched:\n                cxx1.RecordDeclaration.1 : 103\n             cxx1.TypeAliasDeclaration.2 : 11\n                            cxx1.QName.1 : 8\n              cxx1.VariableDeclaration.2 : 7\n                  cxx1.EnumDeclaration.1 : 7\n                             cxx1.Name.1 : 1\n')),(0,i.mdx)("p",null,"If your query is expensive, then likely you will see some large numbers next to one or more predicates. This is a sign that you probably want to reorder the statements in your query, or lift out some nested queries into statements so that you can control the ordering more precisely."),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},"> :debug on\n")),(0,i.mdx)("h2",{id:"showing-the-internals"},"Showing the internals"),(0,i.mdx)("p",null,"The shell provides ways to show what Glean's query engine is doing internally. This is mostly useful for those working on the query engine itself, but it might also be helpful when debugging queries."),(0,i.mdx)("div",{className:"admonition admonition-warning alert alert--danger"},(0,i.mdx)("div",{parentName:"div",className:"admonition-heading"},(0,i.mdx)("h5",{parentName:"div"},(0,i.mdx)("span",{parentName:"h5",className:"admonition-icon"},(0,i.mdx)("svg",{parentName:"span",xmlns:"http://www.w3.org/2000/svg",width:"12",height:"16",viewBox:"0 0 12 16"},(0,i.mdx)("path",{parentName:"svg",fillRule:"evenodd",d:"M5.05.31c.81 2.17.41 3.38-.52 4.31C3.55 5.67 1.98 6.45.9 7.98c-1.45 2.05-1.7 6.53 3.53 7.7-2.2-1.16-2.67-4.52-.3-6.61-.61 2.03.53 3.33 1.94 2.86 1.39-.47 2.3.53 2.27 1.67-.02.78-.31 1.44-1.13 1.81 3.42-.59 4.78-3.42 4.78-5.56 0-2.84-2.53-3.22-1.25-5.61-1.52.13-2.03 1.13-1.89 2.75.09 1.08-1.02 1.8-1.86 1.33-.67-.41-.66-1.19-.06-1.78C8.18 5.31 8.68 2.45 5.05.32L5.03.3l.02.01z"}))),"warning")),(0,i.mdx)("div",{parentName:"div",className:"admonition-content"},(0,i.mdx)("p",{parentName:"div"},"We provide no guarantees about this functionality and it might change\nwithout warning."))),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre",className:"language-lang=sh"},"> :debug ir\n")),(0,i.mdx)("p",null,"Shows the internal representation of the query after parsing, name resolution, type checking, and various transformations to simplify it.  In particular, all the nesting has been flattened at this stage, so you can see the exact order of the searches on each predicate, which might help with performance debugging."),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre",className:"language-lang=sh"},"> :debug bytecode\n")),(0,i.mdx)("p",null,"Shows the compiled bytecode for the query. This is what Glean's virtual machine (VM) will execute to perform the query. Probably not all that useful for debugging queries."))}f.isMDXComponent=!0},47596:function(e,n,t){var r=this&&this.__awaiter||function(e,n,t,r){return new(t||(t=Promise))((function(o,i){function a(e){try{u(r.next(e))}catch(n){i(n)}}function c(e){try{u(r.throw(e))}catch(n){i(n)}}function u(e){var n;e.done?o(e.value):(n=e.value,n instanceof t?n:new t((function(e){e(n)}))).then(a,c)}u((r=r.apply(e,n||[])).next())}))};Object.defineProperty(n,"__esModule",{value:!0}),n.getSpecInfo=void 0;const o=t(11737);n.getSpecInfo=function(e){return r(this,void 0,void 0,(function*(){return yield o.call({module:"bloks",api:"getSpecInfo",args:{styleId:e}})}))}},11737:function(e,n){var t=this&&this.__awaiter||function(e,n,t,r){return new(t||(t=Promise))((function(o,i){function a(e){try{u(r.next(e))}catch(n){i(n)}}function c(e){try{u(r.throw(e))}catch(n){i(n)}}function u(e){var n;e.done?o(e.value):(n=e.value,n instanceof t?n:new t((function(e){e(n)}))).then(a,c)}u((r=r.apply(e,n||[])).next())}))};Object.defineProperty(n,"__esModule",{value:!0}),n.call=void 0;let r=!1,o=0;const i={};n.call=function(e){return t(this,void 0,void 0,(function*(){if("staticdocs.thefacebook.com"!==window.location.hostname&&"localhost"!==window.location.hostname)return Promise.reject(new Error("Not running on static docs"));r||(r=!0,window.addEventListener("message",(e=>{if("static-docs-bridge-response"!==e.data.event)return;const n=e.data.id;n in i||console.error(`Recieved response for id: ${n} with no matching receiver`),"response"in e.data?i[n].resolve(e.data.response):i[n].reject(new Error(e.data.error)),delete i[n]})));const n=o++,t=new Promise(((e,t)=>{i[n]={resolve:e,reject:t}})),a={event:"static-docs-bridge-call",id:n,module:e.module,api:e.api,args:e.args},c="localhost"===window.location.hostname?"*":"https://www.internalfb.com";return window.parent.postMessage(a,c),t}))}},24855:function(e,n,t){var r=this&&this.__awaiter||function(e,n,t,r){return new(t||(t=Promise))((function(o,i){function a(e){try{u(r.next(e))}catch(n){i(n)}}function c(e){try{u(r.throw(e))}catch(n){i(n)}}function u(e){var n;e.done?o(e.value):(n=e.value,n instanceof t?n:new t((function(e){e(n)}))).then(a,c)}u((r=r.apply(e,n||[])).next())}))};Object.defineProperty(n,"__esModule",{value:!0}),n.reportFeatureUsage=n.reportContentCopied=void 0;const o=t(11737);n.reportContentCopied=function(e){return r(this,void 0,void 0,(function*(){const{textContent:n}=e;try{yield o.call({module:"feedback",api:"reportContentCopied",args:{textContent:n}})}catch(t){}}))},n.reportFeatureUsage=function(e){return r(this,void 0,void 0,(function*(){const{featureName:n,id:t}=e;console.log("used feature");try{yield o.call({module:"feedback",api:"reportFeatureUsage",args:{featureName:n,id:t}})}catch(r){}}))}},44256:function(e,n,t){var r=this&&this.__createBinding||(Object.create?function(e,n,t,r){void 0===r&&(r=t),Object.defineProperty(e,r,{enumerable:!0,get:function(){return n[t]}})}:function(e,n,t,r){void 0===r&&(r=t),e[r]=n[t]}),o=this&&this.__setModuleDefault||(Object.create?function(e,n){Object.defineProperty(e,"default",{enumerable:!0,value:n})}:function(e,n){e.default=n}),i=this&&this.__importStar||function(e){if(e&&e.__esModule)return e;var n={};if(null!=e)for(var t in e)"default"!==t&&Object.prototype.hasOwnProperty.call(e,t)&&r(n,e,t);return o(n,e),n};Object.defineProperty(n,"__esModule",{value:!0}),n.OssOnly=n.FbInternalOnly=n.isInternal=n.validateFbContentArgs=n.fbInternalOnly=n.fbContent=n.inpageeditor=n.feedback=n.uidocs=n.bloks=void 0,n.bloks=i(t(47596)),n.uidocs=i(t(17483)),n.feedback=i(t(24855)),n.inpageeditor=i(t(27312));const a=["internal","external"];function c(e){return l(e),s()?"internal"in e?u(e.internal):[]:"external"in e?u(e.external):[]}function u(e){return"function"==typeof e?e():e}function l(e){if("object"!=typeof e)throw new Error(`fbContent() args must be an object containing keys: ${a}. Instead got ${e}`);if(!Object.keys(e).find((e=>a.find((n=>n===e)))))throw new Error(`No valid args found in ${JSON.stringify(e)}. Accepted keys: ${a}`);const n=Object.keys(e).filter((e=>!a.find((n=>n===e))));if(n.length>0)throw new Error(`Unexpected keys ${n} found in fbContent() args. Accepted keys: ${a}`)}function s(){try{return Boolean(!1)}catch(e){return console.log("process.env.FB_INTERNAL couldn't be read, maybe you forgot to add the required webpack EnvironmentPlugin config?",e),!1}}n.fbContent=c,n.fbInternalOnly=function(e){return c({internal:e})},n.validateFbContentArgs=l,n.isInternal=s,n.FbInternalOnly=function(e){return s()?e.children:null},n.OssOnly=function(e){return s()?null:e.children}},27312:function(e,n,t){var r=this&&this.__awaiter||function(e,n,t,r){return new(t||(t=Promise))((function(o,i){function a(e){try{u(r.next(e))}catch(n){i(n)}}function c(e){try{u(r.throw(e))}catch(n){i(n)}}function u(e){var n;e.done?o(e.value):(n=e.value,n instanceof t?n:new t((function(e){e(n)}))).then(a,c)}u((r=r.apply(e,n||[])).next())}))};Object.defineProperty(n,"__esModule",{value:!0}),n.submitDiff=void 0;const o=t(11737);n.submitDiff=function(e){return r(this,void 0,void 0,(function*(){const{file_path:n,new_content:t,project_name:r}=e;try{return yield o.call({module:"inpageeditor",api:"createPhabricatorDiffApi",args:{file_path:n,new_content:t,project_name:r}})}catch(i){throw new Error(`Error occurred while trying to submit diff. Stack trace: ${i}`)}}))}},17483:function(e,n,t){var r=this&&this.__awaiter||function(e,n,t,r){return new(t||(t=Promise))((function(o,i){function a(e){try{u(r.next(e))}catch(n){i(n)}}function c(e){try{u(r.throw(e))}catch(n){i(n)}}function u(e){var n;e.done?o(e.value):(n=e.value,n instanceof t?n:new t((function(e){e(n)}))).then(a,c)}u((r=r.apply(e,n||[])).next())}))};Object.defineProperty(n,"__esModule",{value:!0}),n.getApi=n.docsets=void 0;const o=t(11737);n.docsets={BLOKS_CORE:"887372105406659"},n.getApi=function(e){return r(this,void 0,void 0,(function*(){const{name:n,framework:t,docset:r}=e;return yield o.call({module:"uidocs",api:"getApi",args:{name:n,framework:t,docset:r}})}))}}}]);