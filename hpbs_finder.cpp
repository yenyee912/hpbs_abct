#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstdint>

/**
 * @brief BCT Analyzer for 16-bit ARX structures.
 * Designed for academic verification of differential-linear properties.
 * Algo 1 in the paper
 */
class ARX_BCT_Verifier {
public:
    using word_t = uint16_t;
    static constexpr uint32_t MODULO = 65536; // 2^16

    struct Result {
        word_t di, dip, do_val, dop;
        long double probability;
        uint64_t count;
    };

    /**
     * @brief Calculates the BCT entry for a specific differential quartet.
     * Note: To speed up, we limit the search space or use a representative sample.
     */
    Result compute_entry(word_t d_i, word_t d_ip, word_t d_o, word_t d_op, uint32_t sample_limit = 0) {
        uint64_t count = 0;
        uint32_t iterations = 0;

        // If sample_limit is 0, we do a full 2^16 x 2^16 search (very slow)
        // Adjusting loops to be more cache-friendly
        for (uint32_t x = 0; x < MODULO; ++x) {
            if (sample_limit > 0 && x > sample_limit) break; 
            
            for (uint32_t xp = 0; xp < MODULO; ++xp) {
                if (sample_limit > 0 && xp > sample_limit) break;

                iterations++;
                
                // ARX Boomerang System
                word_t x3 = ((((x + xp) % MODULO) ^ d_o) - (xp ^ d_op)) % MODULO;
                word_t x4 = ((((x ^ d_i) + (xp ^ d_ip)) % MODULO) ^ d_o) - (xp ^ d_ip ^ d_op) % MODULO;

                if ((x3 ^ x4) == d_i) {
                    count++;
                }
            }
        }

        long double prob = static_cast<long double>(count) / iterations;
        return {d_i, d_ip, d_o, d_op, prob, count};
    }
};

void print_header() {
    cout << std::left << std::setw(8) << "Alpha" << std::setw(8) << "Alpha'" 
         << std::setw(8) << "Beta" << std::setw(8) << "Beta'" 
         << std::setw(15) << "Prob" << "Count" << endl;
    cout << std::string(55, '-') << endl;
}

int main() {
    ARX_BCT_Verifier verifier;
    auto start = std::chrono::high_resolution_clock::now();

    print_header();

    // Defined search range for the paper's experiments
    uint16_t alpha = 0x0006;
    uint16_t alpha_p = 0x0000;
    
    // Example: Iterating through Beta and Beta' ranges
    for (uint16_t beta = 0; beta <= 0x5; ++beta) {
        for (uint16_t beta_p = 0; beta_p <= 0x5; ++beta_p) {
            
            // Using a sample limit for quick verification
            // Set to 0 for full enumeration
            auto res = verifier.compute_entry(alpha, alpha_p, beta, beta_p, 256);

            if (res.count > 0) {
                cout << std::hex << "0x" << std::setw(4) << res.di << "  "
                     << "0x" << std::setw(4) << res.dip << "  "
                     << "0x" << std::setw(4) << res.do_val << "  "
                     << "0x" << std::setw(4) << res.dop << "  "
                     << std::dec << std::fixed << std::setprecision(6) << res.probability 
                     << "   " << res.count << endl;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "\nTotal Experiment Time: " << elapsed.count() << "s" << endl;

    return 0;
}
