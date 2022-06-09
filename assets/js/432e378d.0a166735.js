"use strict";(self.webpackChunkwebsite=self.webpackChunkwebsite||[]).push([[9519],{3905:function(e,n,t){t.r(n),t.d(n,{MDXContext:function(){return o},MDXProvider:function(){return x},mdx:function(){return N},useMDXComponents:function(){return u},withMDXComponents:function(){return p}});var a=t(67294);function r(e,n,t){return n in e?Object.defineProperty(e,n,{value:t,enumerable:!0,configurable:!0,writable:!0}):e[n]=t,e}function d(){return d=Object.assign||function(e){for(var n=1;n<arguments.length;n++){var t=arguments[n];for(var a in t)Object.prototype.hasOwnProperty.call(t,a)&&(e[a]=t[a])}return e},d.apply(this,arguments)}function l(e,n){var t=Object.keys(e);if(Object.getOwnPropertySymbols){var a=Object.getOwnPropertySymbols(e);n&&(a=a.filter((function(n){return Object.getOwnPropertyDescriptor(e,n).enumerable}))),t.push.apply(t,a)}return t}function i(e){for(var n=1;n<arguments.length;n++){var t=null!=arguments[n]?arguments[n]:{};n%2?l(Object(t),!0).forEach((function(n){r(e,n,t[n])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(t)):l(Object(t)).forEach((function(n){Object.defineProperty(e,n,Object.getOwnPropertyDescriptor(t,n))}))}return e}function m(e,n){if(null==e)return{};var t,a,r=function(e,n){if(null==e)return{};var t,a,r={},d=Object.keys(e);for(a=0;a<d.length;a++)t=d[a],n.indexOf(t)>=0||(r[t]=e[t]);return r}(e,n);if(Object.getOwnPropertySymbols){var d=Object.getOwnPropertySymbols(e);for(a=0;a<d.length;a++)t=d[a],n.indexOf(t)>=0||Object.prototype.propertyIsEnumerable.call(e,t)&&(r[t]=e[t])}return r}var o=a.createContext({}),p=function(e){return function(n){var t=u(n.components);return a.createElement(e,d({},n,{components:t}))}},u=function(e){var n=a.useContext(o),t=n;return e&&(t="function"==typeof e?e(n):i(i({},n),e)),t},x=function(e){var n=u(e.components);return a.createElement(o.Provider,{value:n},e.children)},c={inlineCode:"code",wrapper:function(e){var n=e.children;return a.createElement(a.Fragment,{},n)}},s=a.forwardRef((function(e,n){var t=e.components,r=e.mdxType,d=e.originalType,l=e.parentName,o=m(e,["components","mdxType","originalType","parentName"]),p=u(t),x=r,s=p["".concat(l,".").concat(x)]||p[x]||c[x]||d;return t?a.createElement(s,i(i({ref:n},o),{},{components:t})):a.createElement(s,i({ref:n},o))}));function N(e,n){var t=arguments,r=n&&n.mdxType;if("string"==typeof e||r){var d=t.length,l=new Array(d);l[0]=s;var i={};for(var m in n)hasOwnProperty.call(n,m)&&(i[m]=n[m]);i.originalType=e,i.mdxType="string"==typeof e?e:r,l[1]=i;for(var o=2;o<d;o++)l[o]=t[o];return a.createElement.apply(null,l)}return a.createElement.apply(null,t)}s.displayName="MDXCreateElement"},16459:function(e,n,t){t.r(n),t.d(n,{contentTitle:function(){return o},default:function(){return c},frontMatter:function(){return m},metadata:function(){return p},toc:function(){return u}});var a=t(87462),r=t(63366),d=(t(67294),t(3905)),l=t(44256),i=["components"],m={id:"thrift",title:"Thrift and JSON",sidebar_label:"Thrift and JSON"},o=void 0,p={unversionedId:"schema/thrift",id:"schema/thrift",isDocsHomePage:!1,title:"Thrift and JSON",description:"The Glean schema is automatically translated into a set of Thrift type",source:"@site/docs/schema/thrift.md",sourceDirName:"schema",slug:"/schema/thrift",permalink:"/docs/schema/thrift",editUrl:"https://github.com/facebookincubator/Glean/tree/main/glean/website/docs/schema/thrift.md",tags:[],version:"current",frontMatter:{id:"thrift",title:"Thrift and JSON",sidebar_label:"Thrift and JSON"},sidebar:"someSidebar",previous:{title:"Workflow",permalink:"/docs/schema/workflow"},next:{title:"Overview",permalink:"/docs/query/intro"}},u=[],x={toc:u};function c(e){var n=e.components,t=(0,r.Z)(e,i);return(0,d.mdx)("wrapper",(0,a.Z)({},x,t,{components:n,mdxType:"MDXLayout"}),(0,d.mdx)("p",null,"The Glean schema is automatically translated into a set of Thrift type\ndefinitions by the ",(0,d.mdx)("inlineCode",{parentName:"p"},"gen-schema")," tool (see ",(0,d.mdx)("a",{parentName:"p",href:"/docs/schema/workflow"},"Workflow"),").\nThese Thrift definitions can be used to work with Glean data in your\nclient, as native data types in whatever language you're using, either\nfor querying data or for writing facts."),(0,d.mdx)("p",null,"The Thrift types also have a JSON representation, which can be read\nand written directly. When you perform queries in the\n",(0,d.mdx)("a",{parentName:"p",href:"/docs/shell"},"shell"),", the results are printed as JSON-encoded Thrift;\nwhen you ",(0,d.mdx)("a",{parentName:"p",href:"/docs/write"},"write data to Glean")," it can be in the form of\nJSON-encoded Thrift."),(0,d.mdx)(l.FbInternalOnly,{mdxType:"FbInternalOnly"},(0,d.mdx)("p",null,"Facebook internal: the Thrift types for the schema are automatically\ngenerated into\n",(0,d.mdx)("a",{parentName:"p",href:"https://phabricator.intern.facebook.com/diffusion/FBS/browse/master/fbcode/glean/schema"},"fbcode/glean/schema"),", and those files are automatically sync'd to\nwww too.")),(0,d.mdx)("p",null,"The relationship between schema types and Thrift/JSON is given by the following table:"),(0,d.mdx)("table",null,(0,d.mdx)("thead",{parentName:"table"},(0,d.mdx)("tr",{parentName:"thead"},(0,d.mdx)("th",{parentName:"tr",align:null},"Schema type"),(0,d.mdx)("th",{parentName:"tr",align:null},"Thrift type"),(0,d.mdx)("th",{parentName:"tr",align:null},"JSON"))),(0,d.mdx)("tbody",{parentName:"table"},(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"nat")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"Nat")," (",(0,d.mdx)("inlineCode",{parentName:"td"},"i64"),")"),(0,d.mdx)("td",{parentName:"tr",align:null},"123")),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"byte")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"Byte")," (",(0,d.mdx)("inlineCode",{parentName:"td"},"i8"),")"),(0,d.mdx)("td",{parentName:"tr",align:null},"123")),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"string")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"string")),(0,d.mdx)("td",{parentName:"tr",align:null},'"abc"')),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"bool")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"bool")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"true")," or ",(0,d.mdx)("inlineCode",{parentName:"td"},"false"))),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"[byte]")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"binary")),(0,d.mdx)("td",{parentName:"tr",align:null},"base-64 encoded string ",(0,d.mdx)("sup",null,"*1"))),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"[T]")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"list<T>")),(0,d.mdx)("td",{parentName:"tr",align:null},"[...]")),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"{"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"f\u2081 : T\u2081,"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"...,"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"f\u2099 : T\u2099"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"struct Foo {"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"1: T\u2081 f\u2081;"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"..."),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"n: T\u2099 f\u2099;"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"{"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},'"f\u2081" : q\u2081,'),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"..."),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},'"f\u2099" : q\u2099'),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}"))),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"{"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"f\u2081 : T\u2081 "),(0,d.mdx)("code",null,"|"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"... "),(0,d.mdx)("code",null,"|"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"f\u2099 : T\u2099"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"union Foo {"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"1: T\u2081 f\u2081;"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"..."),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"n: T\u2099 f\u2099;"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},'{ "f" : t }'),(0,d.mdx)("br",null),"for one of the fields ",(0,d.mdx)("inlineCode",{parentName:"td"},"f\u2081"),"..",(0,d.mdx)("inlineCode",{parentName:"td"},"f\u2099"))),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"maybe T")),(0,d.mdx)("td",{parentName:"tr",align:null},"In a record field:",(0,d.mdx)("br",null)," ",(0,d.mdx)("inlineCode",{parentName:"td"},"optional T f")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"f : t"),(0,d.mdx)("br",null)," if the value is present")),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"enum {"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"L\u2081"),(0,d.mdx)("code",null,"|"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"..."),(0,d.mdx)("code",null,"|"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"L\u2099"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"enum Foo { "),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"L\u2081 = 1,"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"..."),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"L\u2099 = n"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}")),(0,d.mdx)("td",{parentName:"tr",align:null},"the index of the value,",(0,d.mdx)("br",null)," e.g. 12")),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"predicate P : K -> V")),(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"struct P {"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"1: Id id"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"2: optional K key"),(0,d.mdx)("br",null),"\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},"3: optional V value"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"}"),(0,d.mdx)("br",null),"note",(0,d.mdx)("sup",null,"*2")),(0,d.mdx)("td",{parentName:"tr",align:null},"refer to fact N:",(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"N")," or ",(0,d.mdx)("inlineCode",{parentName:"td"},'{ "id": N }'),(0,d.mdx)("br",null),"define a fact:",(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},'{ "id" : N,'),(0,d.mdx)("br",null),"\xa0","\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},'"key" : t }')," or",(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},'{ "key": t }')," or",(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},'{ "key": t,'),(0,d.mdx)("br",null),"\xa0","\xa0","\xa0","\xa0",(0,d.mdx)("inlineCode",{parentName:"td"},'"value" : v }'))),(0,d.mdx)("tr",{parentName:"tbody"},(0,d.mdx)("td",{parentName:"tr",align:null},(0,d.mdx)("inlineCode",{parentName:"td"},"type N = T")),(0,d.mdx)("td",{parentName:"tr",align:null},"depending on T: ",(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"struct N { .. }"),(0,d.mdx)("br",null)," ",(0,d.mdx)("inlineCode",{parentName:"td"},"union N {...}"),(0,d.mdx)("br",null)," ",(0,d.mdx)("inlineCode",{parentName:"td"},"enum N {...}"),(0,d.mdx)("br",null),(0,d.mdx)("inlineCode",{parentName:"td"},"typedef T N;")),(0,d.mdx)("td",{parentName:"tr",align:null},"same as type T")))),(0,d.mdx)("ol",null,(0,d.mdx)("li",{parentName:"ol"},(0,d.mdx)("p",{parentName:"li"},"The Thrift encoding of a binary field in JSON is a base-64-encoded string. However, not all Thrift implementations respect this. At the time of writing, the Python Thrift implementation doesn't base-64-encode binary values. For this reason we provide an option in the Glean Thrift API to disable base-64 encoding for binary if your client doesn't support it. The Glean Shell also uses this option to make it easier to work with binary.")),(0,d.mdx)("li",{parentName:"ol"},(0,d.mdx)("p",{parentName:"li"},"the ",(0,d.mdx)("inlineCode",{parentName:"p"},"key")," is optional - a nested fact may be expanded in place or represented by a reference to the fact ID only. When querying Glean data the query specifies which nested facts should be expanded in the result, and when writing data to Glean using Thrift or JSON, we can optionally specify the value of nested facts inline."))))}c.isMDXComponent=!0}}]);