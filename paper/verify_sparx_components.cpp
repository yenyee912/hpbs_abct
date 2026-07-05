// Experimental verification for the practical SPARX-64 component trails used
// in the HPBS paper. The lower component starts at SPARX round 1, matching the
// X01/Y01 -> X07/Y07 indexing used in the CryptoSMT YAML file.

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using Word = uint16_t;
using State = std::array<Word, 4>;

static inline Word rotl16(Word x, unsigned r) {
    return static_cast<Word>((x << r) | (x >> (16 - r)));
}

static void speckey_round(Word &x, Word &y) {
    x = rotl16(x, 9);
    x = static_cast<Word>(x + y);
    y = static_cast<Word>(rotl16(y, 2) ^ x);
}

static void l2(Word &x0, Word &x1) {
    Word tmp = rotl16(static_cast<Word>(x0 ^ x1), 8);
    x0 = static_cast<Word>(x0 ^ tmp);
    x1 = static_cast<Word>(x1 ^ tmp);
}

static void sparx_rounds(State &s, int rounds, int start_round, bool omit_last_l2) {
    for (int r = start_round; r < start_round + rounds; r++) {
        speckey_round(s[0], s[1]);
        speckey_round(s[2], s[3]);

        bool is_last_round = r == start_round + rounds - 1;
        if ((r + 1) % 3 == 0 && !(omit_last_l2 && is_last_round)) {
            Word lx0 = s[0];
            Word lx1 = s[1];
            l2(lx0, lx1);

            State next{};
            next[0] = static_cast<Word>(lx0 ^ s[2]);
            next[1] = static_cast<Word>(lx1 ^ s[3]);
            next[2] = s[0];
            next[3] = s[1];
            s = next;
        }
    }
}

static State xor_state(const State &a, const State &b) {
    return {
        static_cast<Word>(a[0] ^ b[0]),
        static_cast<Word>(a[1] ^ b[1]),
        static_cast<Word>(a[2] ^ b[2]),
        static_cast<Word>(a[3] ^ b[3]),
    };
}

static State random_state(std::mt19937_64 &rng) {
    uint64_t v = rng();
    return {
        static_cast<Word>(v >> 48),
        static_cast<Word>(v >> 32),
        static_cast<Word>(v >> 16),
        static_cast<Word>(v),
    };
}

static bool equal_state(const State &a, const State &b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static void print_state(const State &x) {
    std::ios old_state(nullptr);
    old_state.copyfmt(std::cout);
    std::cout << std::hex << std::setfill('0')
              << "0x" << std::setw(4) << x[0] << " "
              << "0x" << std::setw(4) << x[1] << " "
              << "0x" << std::setw(4) << x[2] << " "
              << "0x" << std::setw(4) << x[3];
    std::cout.copyfmt(old_state);
}

struct Trail {
    std::string name;
    int rounds;
    int start_round;
    bool omit_last_l2;
    State input_diff;
    State output_diff;
    double expected_log2_probability;
};

static void verify_trail(const Trail &trail, uint64_t trials, uint64_t seed) {
    std::mt19937_64 rng(seed);
    uint64_t successes = 0;

    for (uint64_t i = 0; i < trials; i++) {
        State p1 = random_state(rng);
        State p2 = xor_state(p1, trail.input_diff);
        sparx_rounds(p1, trail.rounds, trail.start_round, trail.omit_last_l2);
        sparx_rounds(p2, trail.rounds, trail.start_round, trail.omit_last_l2);
        if (equal_state(xor_state(p1, p2), trail.output_diff)) {
            successes++;
        }
    }

    double observed = static_cast<double>(successes) / static_cast<double>(trials);
    double observed_log2 = successes == 0 ? -INFINITY : std::log2(observed);
    double expected = std::pow(2.0, trail.expected_log2_probability);
    double expected_successes = static_cast<double>(trials) * expected;

    std::cout << trail.name << "\n";
    std::cout << "  rounds: " << trail.rounds << "\n";
    std::cout << "  start round: " << trail.start_round << "\n";
    std::cout << "  omit last L2: " << (trail.omit_last_l2 ? "yes" : "no") << "\n";
    std::cout << "  input difference: ";
    print_state(trail.input_diff);
    std::cout << "\n  output difference: ";
    print_state(trail.output_diff);
    std::cout << "\n";
    std::cout << "  trials: " << trials << "\n";
    std::cout << "  successes: " << successes << "\n";
    std::cout << "  expected successes: " << expected_successes << "\n";
    std::cout << "  observed probability: " << observed << " = 2^" << observed_log2 << "\n";
    std::cout << "  predicted probability: 2^" << trail.expected_log2_probability << "\n\n";
}

int main(int argc, char **argv) {
    uint64_t trials = 1ULL << 26;
    uint64_t seed = 0x53504152585f4850ULL;

    if (argc > 1) {
        trials = std::strtoull(argv[1], nullptr, 0);
    }
    if (argc > 2) {
        seed = std::strtoull(argv[2], nullptr, 0);
    }

    const std::vector<Trail> trails = {
        {
            "SPARX-64 upper E0 component from the 13-round rectangle trail",
            6,
            0,
            false,
            {0x0000, 0x0000, 0x1005, 0x02a1},
            {0x0245, 0x0747, 0x0040, 0x0542},
            -15.0,
        },
        {
            "SPARX-64 upper E0 component before the final L2 layer",
            6,
            0,
            true,
            {0x0000, 0x0000, 0x1005, 0x02a1},
            {0x0040, 0x0542, 0x0000, 0x0000},
            -15.0,
        },
        {
            "SPARX-64 lower E1 component from the 13-round rectangle trail",
            6,
            1,
            false,
            {0x0000, 0x0000, 0x0a20, 0x4205},
            {0x8004, 0x8c0e, 0x8000, 0x840a},
            -18.0,
        },
    };

    std::cout << "Seed: " << seed << "\n";
    std::cout << "Trials: " << trials << "\n\n";

    for (const auto &trail : trails) {
        verify_trail(trail, trials, seed);
    }

    return 0;
}
