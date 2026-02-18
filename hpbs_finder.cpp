#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstdint>

using namespace std;

// Using standard fixed-width types for portability in papers
typedef uint16_t word_t; 

/**
 * @brief Logic for ARX Boomerang Connectivity Table (BCT) verification.
 * This preserves the original mathematical relationship for x3 and x4.
 */
class BCT_Experiment {
public:
    static constexpr int SIZE = 65536; // 2^16

    struct Result {
        long double probability;
        uint64_t count;
    };

    /**
     * @param limit: Optional sample limit for partial enumeration (your 1/4 idea)
     */
    static Result run_analysis(word_t d_i, word_t d_ip, word_t d_o, word_t d_op, uint32_t limit = SIZE) {
        uint64_t count = 0;
        uint64_t iterations = 0;

        for (uint32_t x = 0; x < limit; x++) {
            // Early break logic preserved from your original snippet if needed
            // if (count > 10) break; 

            for (uint32_t xp = 0; xp < limit; xp++) {
                iterations++;

                // Original Logic: x3 = ((x + xp) ^ n0) - (xp ^ np)
                word_t x3 = (static_cast<word_t>(x + xp) ^ d_o) - (xp ^ d_op);
                
                // Original Logic: x4 = (((x ^ d0) + (xp ^ dp)) ^ n0) - (xp ^ dp ^ np)
                word_t x4 = (static_cast<word_t>((x ^ d_i) + (xp ^ d_ip)) ^ d_o) - (xp ^ d_ip ^ d_op);

                if ((x3 % SIZE) == ((x4 ^ d_i) % SIZE)) {
                    count++;
                }
            }
        }

        // Probability calculation based on the actual sample size used (iterations)
        long double prob = (iterations > 0) ? static_cast<long double>(count) / iterations : 0;
        return {prob, count};
    }
};

int main() {
    // Experiment parameters
    const word_t alpha = 0x0006;
    const word_t alpha_prime = 0x0000;
    
    cout << "BCT Verification Experiment" << endl;
    cout << "Alpha: 0x" << hex << alpha << ", Alpha': 0x" << alpha_prime << dec << endl;
    cout << "--------------------------------------------------------" << endl;
    cout << "Beta\tBeta'\tCount\tProbability" << endl;

    auto start_time = chrono::high_resolution_clock::now();

    // Nested loops for Beta (i) and Beta' (j)
    for (word_t beta = 0; beta <= 0xf; ++beta) {
        for (word_t beta_p = 0; beta_p <= 0xf; ++beta_p) {
            
            // Running your logic
            auto res = BCT_Experiment::run_analysis(alpha, alpha_prime, beta, beta_p);

            if (res.count > 0) {
                cout << hex << "0x" << beta << "\t" 
                     << "0x" << beta_p << "\t" 
                     << dec << res.count << "\t" 
                     << scientific << setprecision(4) << res.probability << endl;
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    
    cout << "--------------------------------------------------------" << endl;
    cout << "Total runtime: " << fixed << setprecision(2) << elapsed.count() << "s" << endl;

    return 0;
}
