#include <emscripten/emscripten.h>
#include <wasm_simd128.h>
#include <vector>
#include <cstdint>
#include <algorithm>

extern "C" {


EMSCRIPTEN_KEEPALIVE
void adjustBrightnessSIMD(uint8_t *data, int length, int brightness) {
    int i = 0;

    // Use unsigned saturating ops for pixel bytes (0..255).
    if (brightness >= 0) {
        v128_t bright = wasm_u8x16_splat((uint8_t)brightness);
        for (; i <= length - 16; i += 16) {
            v128_t pixels = wasm_v128_load(&data[i]);
            v128_t result = wasm_u8x16_add_sat(pixels, bright);
            wasm_v128_store(&data[i], result);
        }
    } else {
        // Negative brightness => saturating subtract
        uint8_t b = (uint8_t)(-brightness);
        v128_t bright = wasm_u8x16_splat(b);
        for (; i <= length - 16; i += 16) {
            v128_t pixels = wasm_v128_load(&data[i]);
            v128_t result = wasm_u8x16_sub_sat(pixels, bright);
            wasm_v128_store(&data[i], result);
        }
    }

    // Tail scalar for remaining bytes
    for (; i < length; i++) {
        int val = data[i] + brightness;
        data[i] = static_cast<uint8_t>(std::min(255, std::max(0, val)));
    }
}
}

// #include <iostream>
// #include <opencv2/opencv.hpp>
// #include <arm_neon.h>
// #include <chrono>

// using namespace std;
// using namespace cv;

// void adjustBrightnessScalar(Mat &image, int brightness) {
//     for (int i = 0; i < image.rows; i++) {
//         uchar *row = image.ptr<uchar>(i);
//         for (int j = 0; j < image.cols * image.channels(); j++) {
//             int val = row[j] + brightness;
//             row[j] = saturate_cast<uchar>(val);
//         }
//     }
// }
// void adjustBrightnessSIMD(Mat &image, int brightness) {
//     int totalPixels = image.rows * image.cols * image.channels();
//     uchar *data = image.data;

//     // Use ARM NEON 128-bit registers (process 16 bytes per iteration)
//     uint8x16_t bright = vdupq_n_u8((uint8_t)brightness);
//     int i = 0;
//     for (; i <= totalPixels - 16; i += 16) {
//         uint8x16_t pixels = vld1q_u8(&data[i]);
//         uint8x16_t result = vqaddq_u8(pixels, bright); // saturated add
//         vst1q_u8(&data[i], result);
//     }
//     // Handle leftover pixels
//     for (; i < totalPixels; i++) {
//         int val = data[i] + brightness;
//         data[i] = saturate_cast<uchar>(val);
//     }
// }
// double measureTime(function<void()> func) {
//     auto start = chrono::high_resolution_clock::now();
//     func();
//     auto end = chrono::high_resolution_clock::now();
//     chrono::duration<double> diff = end - start;
//     return diff.count();
// }

// int main()
// {
//     string imagePath = "sample.jpg";
//     Mat img = imread(imagePath);

//     if (img.empty()) {
//         cout << "Error: Could not load image!" << endl;
//         return -1;
//     }
//     cout << "Image Loaded: " << img.cols << "x" << img.rows << endl;
//     Mat imgScalar = img.clone();
//     Mat imgSIMD = img.clone();
//     int brightness = 40;
//     double scalarTime = measureTime([&]() { adjustBrightnessScalar(imgScalar, brightness); });
//     double simdTime = measureTime([&]() { adjustBrightnessSIMD(imgSIMD, brightness); });

//     cout << "\n=== PERFORMANCE COMPARISON ===" << endl;
//     cout << "Scalar: " << scalarTime << " sec" << endl;
//     cout << "SIMD:   " << simdTime << " sec" << endl;
//     cout << "Speedup: " << scalarTime / simdTime << "x faster" << endl;

//     imshow("Original Image", img);
//     imshow("Scalar Brightness", imgScalar);
//     imshow("SIMD Brightness", imgSIMD);
//     waitKey(0);
// }