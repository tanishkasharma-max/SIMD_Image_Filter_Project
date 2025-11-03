# SIMD Image Brightness Filter (WASM)

Repository layout
- simd_image_filter.cpp — C++ implementations:
  - `adjustBrightnessScalar(uint8_t*, int, int)`
  - `adjustBrightnessSIMD(uint8_t*, int, int)` (uses `wasm_simd128.h`)
- simd_image_filter.js / simd_image_filter.wasm — generated Emscripten outputs (must be built and placed next to index.html)
- index.html — UI: image loader (scaled to 200×200), JS scalar path, WASM integration, diagnostics


Design
- WASM runs native-like code in browsers; SIMD processes 16 bytes per instruction to speed up pixel ops.
- WASM uses its own linear memory (heap). To operate on image bytes you must copy data into the heap and pass a pointer to C/WASM.
- Alpha handling: C code operates on raw bytes; index.html preserves alpha in JS to avoid visual artifacts.

Build (quick emcc command)
- Example one-line build (use project root):
```bash
emcc simd_image_filter.cpp -O3 -msimd128 \
  -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=134217728 \
  -s EXPORTED_FUNCTIONS="['_adjustBrightnessScalar','_adjustBrightnessSIMD','_malloc','_free']" \
  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
  -o simd_image_filter.js
```
- Notes:
  - `-msimd128` enables wasm SIMD support.
  - `EXPORTED_FUNCTIONS` ensures your C functions and malloc/free are callable from JS.
  - `EXPORTED_RUNTIME_METHODS` exposes `ccall`/`cwrap` used by index.html.
  - `ALLOW_MEMORY_GROWTH=1` and `TOTAL_MEMORY` avoid heap-size errors for larger images.

Build (CMake)
- If using CMake/emcmake, add similar linker flags for the Emscripten target:
```cmake
set_target_properties(simd_image_filter PROPERTIES
  LINK_FLAGS "-O3 -msimd128 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=134217728 \
  -s EXPORTED_FUNCTIONS=\"['_adjustBrightnessScalar','_adjustBrightnessSIMD','_malloc','_free']\" \
  -s EXPORTED_RUNTIME_METHODS=\"['ccall','cwrap']\""
)
```

How the page works (data flow)
1. Load image → canvas scaled to 200×200 → get ImageData (RGBA).
2. JS scalar: modifies RGB bytes in place (preserves alpha) and measures time.
3. WASM path:
   - Allocate buffer in wasm heap (`_malloc`).
   - Copy image bytes into heap (`Module.HEAPU8.set`).
   - Call C function via `Module.ccall(func, ...)`.
   - Read back modified bytes and update canvas (restore alpha).
   - Measure and display timing.

Common issues and fixes
- "WASM runtime missing required functions (_malloc/ccall)"
  - Rebuild with `EXPORTED_RUNTIME_METHODS` and `_malloc/_free` in `EXPORTED_FUNCTIONS`.
- "WASM heap unavailable or too small"
  - Rebuild with `-s ALLOW_MEMORY_GROWTH=1` and increase `-s TOTAL_MEMORY`.
  - Ensure the served simd_image_filter.js is the new build (clear cache).
- `adjustBrightnessSIMD` has no effect
  - Ensure function exported (`EXPORTED_FUNCTIONS`) and compiled with `-msimd128`.
  - Verify browser supports WASM SIMD (modern Chrome/Edge/Firefox).



Performance / correctness notes
- SIMD processes 16 bytes at a time with saturating ops; tail bytes handled in scalar loop.
- Copying to/from wasm heap is necessary and dominates small images; for larger images SIMD shows benefits.
- To reduce overhead, preallocate a single wasm buffer and reuse it instead of malloc/free per call.

Scalar Js: 2.50 ms <br>
WASM : 1.40 ms 



