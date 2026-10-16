// ARM64 GCC -00 output: https://godbolt.org/z/TfhsaTroY
// ARM64 GCC -03 output: https://godbolt.org/z/8qx7K6qfG

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
    switch (argc) {                                         // Parse command line args for size of vector
        case 2:
            N = stoull(argv[1]);
            break;
        default:
            break;
    }

    vector<float> A(N), B(N), C(N);                         // We want to add the elements of A and B into C
    random_vector(A, N);
    random_vector(B, N);

    auto start = chrono::steady_clock::now();
    for (size_t rep = 0; rep < REPTITIONS; rep++) {
        do_not_optimize(A.data());                          // force re-read of array at each step
        do_not_optimize(B.data());
        for (size_t i = 0; i < N; i++) {
            C[i] = A[i] + B[i];
        }
        do_not_optimize(C.data());                          // force recomputation of sum
    }
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Elapsed: " << elapsed.count() << "ms" << endl;
    return 0;
}

