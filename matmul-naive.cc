// ARM64 GCC -O0 output: https://godbolt.org/z/sKWTP7q5T
// ARM64 GCC -O3 output: https://godbolt.org/z/YEf896b3a

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
void matmul_naive(const vector<vector<float>>& A, const vector<vector<float>>& B, vector<vector<float>>& C) {
    size_t N = A.size();
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            for (size_t k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    size_t N = 1000;
    switch (argc) {                                                 // Parse command line args for size of matrix
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
    matmul_naive(A, B, C);
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Elapsed: " << elapsed.count() << "ms" << endl;
    return 0;
}

