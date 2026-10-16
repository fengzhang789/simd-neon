// ARM64 GCC -03 output: https://godbolt.org/z/5bYhE5rKz

#include <random>
#include <chrono>
#include <iostream>
#include <arm_neon.h>
#include <cstdint>

using namespace std;

// Helper function that outputs empty assembly and prevents some compiler optimizations
// for variable v
template <typename T>
inline void do_not_optimize(T const& v) {
    asm volatile("" : : "r"(v) : "memory");
}

int main() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distrib(1, 255);

    const int N = 1e5;
    int A[N];
    
    for (int& x : A) {
        x = distrib(gen);
    }

    auto start = chrono::steady_clock::now();
    int64_t sum = 0;
    for (int rep=0; rep<N; rep++) {
        do_not_optimize(&A[0]);                                 // force re-read of array at each step
        int32x4_t accumulator = vdupq_n_s32(0);                 // set all lanes to 0 for this accumulator
        for (int i = 0; i + 4 <= N; i += 4) {
            int32x4_t vector = vld1q_s32(&A[i]);                // set vector to be the next 4 elements of A
            accumulator      = vaddq_s32(vector, accumulator);  // add vector to accumulator
        }
        sum = vaddlvq_s32(accumulator);                         // sum up all 4 32-bit numbers in accumulator and widen to long long
        do_not_optimize(sum);                                   // force recomputation of sum
    }
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Elapsed: " << elapsed.count() << "ms" << endl;
    cout << "Sum: " << sum << endl;
    return 0;
}

