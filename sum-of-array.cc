// ARM64 GCC -03 output: https://godbolt.org/z/Yvaqo1G5a
// ARM64 GCC -00 output: https://godbolt.org/z/zr3bKhhc8

#include <random>
#include <chrono>
#include <iostream>

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
    long long sum = 0;
    for (int rep=0; rep<N; rep++) {
        do_not_optimize(&A[0]);                     // force re-read of array at each step
        long long s = 0;
        for (int i=0; i<N; i++) {
            s += A[i];
        }
        do_not_optimize(s);                         // force recomputation of sum
        sum = s;
    }
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Elapsed: " << elapsed.count() << "ms" << endl;
    cout << "Sum: " << sum << endl;
    return 0;
}
