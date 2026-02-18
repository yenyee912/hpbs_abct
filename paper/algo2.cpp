#include <iostream>
#include <random>
#include <vector>
#include <iomanip>

using namespace std;

typedef uint16_t word_t;
const uint32_t POOL_SIZE = 65536;

// Core BCT Check for 16-bit word
bool check_bct_16bit(word_t d_i, word_t d_ip, word_t d_o, word_t d_op) {
    int count = 0;
    // Empirical search in the 16-bit space
    for (uint32_t x = 0; x < 2048; x++) { // Sufficient sampling for high-prob patterns
        if (count > 1) return true;
        for (uint32_t xp = 0; xp < 64; xp++) {
            word_t x3 = ((((x + xp) % POOL_SIZE) ^ d_o) - (xp ^ d_op)) % POOL_SIZE;
            word_t x4 = ((((x ^ d_i) + (xp ^ d_ip)) % POOL_SIZE) ^ d_o) - (xp ^ d_ip ^ d_op) % POOL_SIZE;

            if ((x3 ^ x4) == d_i) count++;
        }
    }
    return count > 0;
}

int main() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> upper_dist(0, 0x0FFF); // Randomizing upper 12 bits

    word_t alpha = 0x0;
    word_t alpha_p = 0x2;

    cout << "Empirical Validation for Alpha=0, Alpha'=2" << endl;
    cout << "Condition: 2nd LSB(beta) == 2nd LSB(beta')" << endl;
    cout << "------------------------------------------------" << endl;
    cout << "Beta\tBeta'\tMatch?\tSuccess Rate (N=100)" << endl;

    for (word_t b_lsb = 0; b_lsb <= 0xF; b_lsb++) {
        // Test two cases for each Beta: 
        // 1. A Beta' that matches the 2nd LSB (should succeed)
        // 2. A Beta' that flips the 2nd LSB (should fail)
        
        word_t bp_valid_lsb = b_lsb; // Matches 2nd LSB
        word_t bp_invalid_lsb = b_lsb ^ 0x2; // Flips 2nd LSB

        word_t test_bp[] = {bp_valid_lsb, bp_invalid_lsb};

        for (word_t bp_lsb : test_bp) {
            int success_count = 0;
            int N = 100; 

            for (int i = 0; i < N; i++) {
                // Construct 16-bit words: [Random 12 bits] + [4-bit Pattern]
                word_t a  = (upper_dist(gen) << 4) | (alpha & 0xF);
                word_t ap = (upper_dist(gen) << 4) | (alpha_p & 0xF);
                word_t b  = (upper_dist(gen) << 4) | (b_lsb & 0xF);
                word_t bp = (upper_dist(gen) << 4) | (bp_lsb & 0xF);

                if (check_bct_16bit(a, ap, b, bp)) success_count++;
            }

            bool matches = ((b_lsb & 0x2) == (bp_lsb & 0x2));
            
            cout << hex << b_lsb << "\t" << bp_lsb << "\t" 
                 << (matches ? "YES" : "NO") << "\t" 
                 << dec << success_count << "%" << endl;
        }
        cout << "------------------------------------------------" << endl;
    }

    return 0;
}
