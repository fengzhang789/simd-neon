// ARM64 GCC -O3 output: https://godbolt.org/z/bWfd6evv9

#include <arm_neon.h>
#include <random>
#include <chrono>
#include <iostream>
#include <vector>

using namespace std;

// Helper function that outputs empty assembly and prevents some compiler optimizations
// for variable v
template <typename T>
inline void do_not_optimize(T const& v) {
    asm volatile("" : : "r"(v) : "memory");
}

// Generates a matrix of size n x n with random float entries between -4 and 4
void random_matrix(vector<vector<float>>& v, size_t n) {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<float> distrib(-4.0f, 4.0f);

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            v[i][j] = distrib(gen);
        }
    }
}

// matrix multiplication for n x n matrix
// O(N * N *N) TC
// requires: C is initialized as n x n zero matrix
void matmul_transposed_simd(const vector<vector<float>>& A, const vector<vector<float>>& B, vector<vector<float>>& C) {
    // transpose B first, in O(n^2) time
    size_t N = A.size();
    vector<vector<float>> B_transpose(N, vector<float>(N));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            B_transpose[j][i] = B[i][j];
        }
    }

    // now row(A, i) * row(B_transpose, j) is C[i][j]
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            float32x4_t accumulator = vdupq_n_f32(0.0f);                        // initialize sum to 0
            size_t k = 0;
            for (; k + 4 <= N; k += 4) {
                float32x4_t A_vector = vld1q_f32(A[i].data() + k);              // load
                float32x4_t B_vector = vld1q_f32(B_transpose[j].data() + k);    // load
                accumulator = vfmaq_f32(accumulator, A_vector, B_vector);       // fused multiply & add to accumulator
            }
            float sum = vaddvq_f32(accumulator);                                // sum across accumulator vector
            for (; k < N; k++) sum += A[i][k] * B_transpose[j][k];              // add remaining sum for non multiples of 4
            C[i][j] = sum;
        }
    }
}

int main(int argc, char* argv[]) {
    size_t N = 1000;
    switch (argc) {                                                             // Parse command line args for size of matrix
        case 2:
            N = stoull(argv[1]);
            break;
        default:
            break;
    }

    vector<vector<float>> A(N, vector<float>(N));
    vector<vector<float>> B(N, vector<float>(N));
    vector<vector<float>> C(N, vector<float>(N, 0));
    random_matrix(A, N);
    random_matrix(B, N);

    auto start = chrono::steady_clock::now();
    matmul_transposed_simd(A, B, C);
    do_not_optimize(C.data());
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Elapsed: " << elapsed.count() << "ms" << endl;
    return 0;
}

