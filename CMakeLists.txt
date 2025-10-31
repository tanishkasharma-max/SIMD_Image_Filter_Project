
cmake_minimum_required(VERSION 3.10)
project(SIMD_ImageFilter)

set(CMAKE_CXX_STANDARD 17)

# --- Detect WebAssembly build ---
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(STATUS "Building for WebAssembly (Emscripten)")

    # Set output to .js (Emscripten produces both .js + .wasm)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    # Add optimization + SIMD flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -msimd128 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1")

    #  Correct way to add source file
    add_executable(simd_image_filter simd_image_filter.cpp)

    # Export functions to JS/WASM runtime
    set_target_properties(simd_image_filter PROPERTIES LINK_FLAGS
        "-s EXPORTED_FUNCTIONS='[_adjustBrightnessScalar,_adjustBrightnessSIMD]' \
         -s EXPORTED_RUNTIME_METHODS='[ccall,cwrap]'"
    )

else()
    message(STATUS "Building for native (macOS/Linux)")

    # Explicitly tell CMake where to find OpenCV
    set(OpenCV_DIR /opt/homebrew/opt/opencv/lib/cmake/opencv4)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})

    # Add SIMD optimization only for Intel builds
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        add_compile_options(-mavx2)
    endif()

    # Correct executable target
    add_executable(simd_image_filter simd_image_filter.cpp)
    target_link_libraries(simd_image_filter ${OpenCV_LIBS})
endif()


# cmake_minimum_required(VERSION 3.10)
# project(SIMD_ImageFilter)

# set(CMAKE_CXX_STANDARD 17)

# # Explicitly tell CMake where to find OpenCV
# set(OpenCV_DIR /opt/homebrew/opt/opencv/lib/cmake/opencv4)

# find_package(OpenCV REQUIRED)
# include_directories(${OpenCV_INCLUDE_DIRS})

# # Add SIMD flag only if x86_64 (not Apple Silicon)
# if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
#     add_compile_options(-mavx2)
# endif()

# add_executable(simd_image_filter brightness.cpp)
# target_link_libraries(simd_image_filter ${OpenCV_LIBS})
