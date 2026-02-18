#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdint>

using namespace std;

typedef uint16_t word_t;
const uint32_t MODULO = 65536;

/**
 * @brief Procedure Quick_ABCT
 * Preserves your exact filtration logic and writes to file if true.
 */
bool quick_abct(word_t a, word_t ap, word_t b, word_t bp, ofstream &outFile) {
    for (word_t i = 0; i <= 0xF; i++) {
        for (word_t j = 0; j <= 0xF; j++) {
            
            // x = (i [+] j) ^ b [-] (j ^ bp)
            word_t term1 = (i + j) % MODULO;
            word_t x = (term1 ^ b) - (j ^ bp);
            x %= MODULO;

            // x' = ((i ^ a) [+] (j ^ ap)) ^ b [-] (j ^ ap ^ bp)
            word_t term2 = ((i ^ a) + (j ^ ap)) % MODULO;
            word_t x_p = (term2 ^ b) - (j ^ ap ^ bp);
            x_p %= MODULO;

            if ((x ^ x_p) == a) {
                // Write to CSV format: alpha, alpha_prime, beta, beta_prime
                outFile << "0x" << hex << a << "," 
                        << "0x" << ap << "," 
                        << "0x" << b << "," 
                        << "0x" << bp << dec << endl;
                return true; 
            }
        }
    }
    return false;
}

int main() {
    ofstream outFile("HPBS_results.csv");
    
    if (!outFile) {
        cerr << "Error: Could not open file for writing." << endl;
        return 1;
    }

    // CSV Header
    outFile << "alpha,alpha_prime,beta,beta_prime" << endl;

    uint32_t total_tested = 0;
    uint32_t total_passed = 0;

    cout << "Analyzing HPBS patterns... Progress logged to HPBS_results.csv" << endl;

    for (word_t a = 0; a <= 0xF; a++) {
        for (word_t ap = 0; ap <= 0xF; ap++) {
            for (word_t b = 0; b <= 0xF; b++) {
                for (word_t bp = 0; bp <= 0xF; bp++) {
                    
                    total_tested++;
                    if (quick_abct(a, ap, b, bp, outFile)) {
                        total_passed++;
                    }
                    
                }
            }
        }
        // Progress indicator for the user
        cout << "Completed Alpha: 0x" << hex << a << dec << endl;
    }

    outFile.close();

    // Summary for your paper's "Experimental Results" section
    cout << "\n--- Experiment Summary ---" << endl;
    cout << "Total Patterns Tested: " << total_tested << endl;
    cout << "Patterns Passed (Hits): " << total_passed << endl;
    cout << "Filtration Rate: " << (static_cast<double>(total_passed) / total_tested) * 100 << "%" << endl;
    cout << "Results saved to HPBS_results.csv" << endl;

    return 0;
}
