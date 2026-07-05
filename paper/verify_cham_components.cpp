// Experimental verification for the practical CHAM-64 component trails used in
// the HPBS paper. The full 36-round boomerang probability is too small for
// direct sampling, so this program verifies the feasible upper/lower
// differentials that feed the boomerang switch.

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
using MasterKey = std::array<Word, 8>;
using RoundKeys = std::array<Word, 16>;

static inline Word rotl16(Word x, unsigned r) {
    return static_cast<Word>((x << r) | (x >> (16 - r)));
}

static void key_schedule(const MasterKey &master, RoundKeys &round_keys) {
    for (int i = 0; i < 8; i++) {
        Word k = master[i];
        round_keys[i] = static_cast<Word>(k ^ rotl16(k, 1) ^ rotl16(k, 8));
        round_keys[(i + 8) ^ 1] = static_cast<Word>(k ^ rotl16(k, 1) ^ rotl16(k, 11));
    }
}

static void cham64_encrypt(State &x, const RoundKeys &round_keys, int rounds) {
    for (int i = 0; i < rounds; i++) {
        State y{};
        y[0] = x[1];
        y[1] = x[2];
        y[2] = x[3];

        Word addend = 0;
        if ((i & 1) == 0) {
            addend = static_cast<Word>(rotl16(x[1], 1) ^ round_keys[i % 16]);
            y[3] = rotl16(static_cast<Word>((x[0] ^ i) + addend), 8);
        } else {
            addend = static_cast<Word>(rotl16(x[1], 8) ^ round_keys[i % 16]);
            y[3] = rotl16(static_cast<Word>((x[0] ^ i) + addend), 1);
        }
        x = y;
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

static MasterKey random_key(std::mt19937_64 &rng) {
    MasterKey key{};
    for (auto &word : key) {
        word = static_cast<Word>(rng());
    }
    return key;
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
    State input_diff;
    State output_diff;
    double expected_log2_probability;
};

static void verify_trail(const Trail &trail, uint64_t trials_per_key, int keys, std::mt19937_64 &rng) {
    uint64_t successes = 0;
    uint64_t total = 0;

    for (int k = 0; k < keys; k++) {
        MasterKey master = random_key(rng);
        RoundKeys round_keys{};
        key_schedule(master, round_keys);

        for (uint64_t i = 0; i < trials_per_key; i++) {
            State p1 = random_state(rng);
            State p2 = xor_state(p1, trail.input_diff);
            cham64_encrypt(p1, round_keys, trail.rounds);
            cham64_encrypt(p2, round_keys, trail.rounds);
            if (equal_state(xor_state(p1, p2), trail.output_diff)) {
                successes++;
            }
            total++;
        }
    }

    double observed = static_cast<double>(successes) / static_cast<double>(total);
    double observed_log2 = successes == 0 ? -INFINITY : std::log2(observed);
    double expected = std::pow(2.0, trail.expected_log2_probability);
    double expected_successes = static_cast<double>(total) * expected;

    std::cout << trail.name << "\n";
    std::cout << "  rounds: " << trail.rounds << "\n";
    std::cout << "  input difference: ";
    print_state(trail.input_diff);
    std::cout << "\n  output difference: ";
    print_state(trail.output_diff);
    std::cout << "\n";
    std::cout << "  trials: " << total << " across " << keys << " key(s)\n";
    std::cout << "  successes: " << successes << "\n";
    std::cout << "  expected successes: " << expected_successes << "\n";
    std::cout << "  observed probability: " << observed << " = 2^" << observed_log2 << "\n";
    std::cout << "  predicted probability: 2^" << trail.expected_log2_probability << "\n\n";
}

int main(int argc, char **argv) {
    uint64_t trials_per_key = 1ULL << 22;
    int keys = 1;
    uint64_t seed = 0x485042535f414243ULL;

    if (argc > 1) {
        trials_per_key = std::strtoull(argv[1], nullptr, 0);
    }
    if (argc > 2) {
        keys = std::atoi(argv[2]);
    }
    if (argc > 3) {
        seed = std::strtoull(argv[3], nullptr, 0);
    }

    std::mt19937_64 rng(seed);

    const std::vector<Trail> trails = {
        {
            "CHAM-64 upper E0 component from the 36-round boomerang trail",
            17,
            {0x8200, 0x0100, 0x0001, 0x8000},
            {0x0400, 0x0004, 0x0502, 0x0088},
            -15.0,
        },
        {
            "CHAM-64 lower E1 component from the 36-round boomerang trail",
            18,
            {0x8200, 0x0100, 0x0001, 0x8000},
            {0x0004, 0x0502, 0x0088, 0x0000},
            -16.0,
        },
    };

    std::cout << "Seed: " << seed << "\n";
    std::cout << "Trials per key: " << trials_per_key << "\n";
    std::cout << "Keys: " << keys << "\n\n";

    for (const auto &trail : trails) {
        verify_trail(trail, trials_per_key, keys, rng);
    }

    return 0;
}
