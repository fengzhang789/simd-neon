// ARM64 GCC -03 output: https://godbolt.org/z/WrnrEKvvW

#include <arm_neon.h>
#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Helper function that outputs empty assembly and prevents some compiler optimizations
// for variable v
template <typename T>
inline void do_not_optimize(T const& v) {
    asm volatile("" : : "r"(v) : "memory");
}

// Generates a vector of size n with random floats
void random_vector(vector<float>& v, size_t n) {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<float> distrib(-10.0f, 10.0f);

    for (size_t i = 0; i < n; i++) {
        v[i] = distrib(gen);
    }
}

int main(int argc, char* argv[]) {
    constexpr size_t REPTITIONS = 1e4;
    size_t N = 1e4;
    switch (argc) {                                                 // Parse command line args for size of vector
        case 2:
            N = stoull(argv[1]);
            break;
        default:
            break;
    }

    vector<float> A(N), B(N), C(N);                                 // We want to add the elements of A and B into C
    random_vector(A, N);
    random_vector(B, N);

    auto start = chrono::steady_clock::now();
    for (size_t rep = 0; rep < REPTITIONS; rep++) {
        do_not_optimize(A.data());                                  // force re-read of array at each step
        do_not_optimize(B.data());
        size_t i = 0;

        for (; i + 4 <= N; i += 4) {
            float32x4_t vector_a = vld1q_f32(A.data() + i);         // load 4 floats at A's current position into vector_a SIMD register
            float32x4_t vector_b = vld1q_f32(B.data() + i);         // load 4 floats at B's current position into vector_b SIMD reigster
            float32x4_t vector_c = vaddq_f32(vector_a, vector_b);   // add vector_a and vector_b
            vst1q_f32(C.data() + i, vector_c);                      // store 4 floats from SIMD register result into C[i]
        }

        // if N is not a multiple of 4, finish tail-end without SIMD
        for (; i < N; i++) {
            C[i] = A[i] + B[i];
        }
        do_not_optimize(C.data());                                  // force recomputation of sum
    }
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Elapsed: " << elapsed.count() << "ms" << endl;
    return 0;
}

