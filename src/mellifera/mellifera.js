/*
Add the following two script tags in the body of your HTML or Markdown document
to add Mellifera support to the associated page:

<script src="/mellifera/wasm/wasm_exec.js"></script>
<script src="/mellifera/mellifera.js"></script>
*/
"use strict";

window.mellifera = {
    /* Evaluate the provided source string. */
    eval: (source, options) => { throw new Error("Mellifera Wasm module is not initialized!"); },
    /* Evaluate the source text in the HTML element with the provided ID. */
    evalById: (id, options) => { mellifera.eval(document.getElementById(id).value, options); },
};

const go = new Go();

// Manually fetch the .wasm file and instantiate with WebAssembly.instantiate
// instead of WebAssembly.instantiateStreaming. Atempting to instante using
// WebAssembly.instantiateStreaming will produce the error:
//
//      TypeError: WebAssembly: Response has unsupported MIME type 'application/octet-stream' expected 'application/wasm'
//
// when deployed on a web server which has not been set up to serve Wasm files
// with the content type application/wasm.
(async () => {
    const melliferaWasmResponse = await fetch("/mellifera/wasm/mellifera.wasm");
    const melliferaWasmBuffer = await melliferaWasmResponse.arrayBuffer();
    WebAssembly.instantiate(melliferaWasmBuffer, go.importObject).then(async result => {
        let module = result.module;
        let instance = result.instance;
        await go.run(instance);
    });
})();
