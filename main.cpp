#include <iostream>
#include <cstdint>
#include <random>
#include <chrono>
#include <vector>
#include <stdexcept>
#include <gmpxx.h>

using namespace std;
using namespace chrono;

const mpz_class BBS_P("D5BBB96D30086EC484EBA3D7F9CAEB07", 16);
const mpz_class BBS_Q("425D2B9BFDB25B9CF6C416CC6E37B59C1F", 16);
const mpz_class BBS_N = BBS_P * BBS_Q;

gmp_randclass& rng() {
    static gmp_randclass r(gmp_randinit_mt);
    static bool seeded = false;

    if (!seeded) {
        r.seed(random_device{}());
        seeded = true;
    }

    return r;
}

mpz_class ModExp(const mpz_class& b,const mpz_class& e, const mpz_class& mod) {
    mpz_class result;
    mpz_powm(result.get_mpz_t(), b.get_mpz_t(), e.get_mpz_t(), mod.get_mpz_t());
    return result;
}

bool Miller_Rabin(const mpz_class& n, int trials = 20) {
    if (n < 2) return false;

    static const unsigned long small_primes[] = {
        2, 3, 5, 7, 11, 13, 17, 19,
        23, 29, 31, 37, 41, 43, 47
    };

    for (unsigned long p : small_primes) {
        if (n == p) return true;
        if (n % p == 0) return false;
    }

    mpz_class d = n - 1;
    int s = 0;

    while (mpz_even_p(d.get_mpz_t())) {
        d >>= 1;
        s++;
    }

    for (int i = 0; i < trials; i++) {
        mpz_class a = rng().get_z_range(n - 3) + 2;
        mpz_class x = ModExp(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool passed = false;

        for (int j = 1; j < s; j++) {
            x = (x * x) % n;
            if (x == n - 1) {passed = true; break;}
            if (x == 1) return false;
        }

        if (!passed) return false;
    }

    return true;
}

uint64_t SplitMix64(uint64_t x) {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;

    return x ^ (x >> 31);
}

mpz_class MakeBigSeed(uint64_t seed, size_t words = 4) {
    vector<uint64_t> values(words);
    for (size_t i = 0; i < words; i++) values[i] = SplitMix64(seed + i);
    mpz_class result;
    mpz_import(result.get_mpz_t(), words, 1, sizeof(uint64_t), 0, 0, values.data());
    return result;
}

mpz_class MakeBBSSeed(uint64_t seed) {
    mpz_class s = MakeBigSeed(seed, 5) % (BBS_N - 1) + 1;
    mpz_class gcd;

    while (true) {
        mpz_gcd(gcd.get_mpz_t(), s.get_mpz_t(), BBS_N.get_mpz_t());
        if (gcd == 1) break;
        ++s;
        if (s >= BBS_N) s = 1;
    }

    return (s * s) % BBS_N;
}

class Blum_Blum_Shub {

private:
    mpz_class N;
    mpz_class x;

public:
    Blum_Blum_Shub(const mpz_class& p, const mpz_class& q, const mpz_class& x0) {
        N = p * q;
        x = x0;
    }

    uint32_t next_bit() {
        x = (x * x) % N;
        return mpz_tstbit(x.get_mpz_t(), 0);
    }
};

mpz_class GenerateCandidate(Blum_Blum_Shub& gen, size_t bits) {
    mpz_class candidate = 0;

    for (size_t i = 0; i < bits; i++) {
        candidate <<= 1;
        candidate += gen.next_bit();
    }

    mpz_setbit(candidate.get_mpz_t(), bits - 1);
    mpz_setbit(candidate.get_mpz_t(), 0);

    return candidate;
}

mpz_class Generate_Prime(size_t bits, uint64_t seed) {
    mpz_class x0 = MakeBBSSeed(seed);
    Blum_Blum_Shub gen(BBS_P, BBS_Q, x0);

    while (true) {
        mpz_class candidate = GenerateCandidate(gen, bits);
        if (Miller_Rabin(candidate, 20)) return candidate;
    }
}





int main() {
    size_t bits;

    cout << "Enter number of bits: ";
    if (!(cin >> bits)) {
        cerr << "Invalid input\n";
        return 1;
    }

    uint64_t clock_seed = static_cast<uint64_t>(high_resolution_clock::now().time_since_epoch().count());
    mt19937_64 seed_engine(clock_seed);

    try {
        mpz_class primea1 = Generate_Prime(bits, seed_engine());
        mpz_class primea2 = Generate_Prime(bits, seed_engine());
        mpz_class primeb1 = Generate_Prime(bits, seed_engine());
        mpz_class primeb2 = Generate_Prime(bits, seed_engine());

        cout << "\nPrime Pair 1:\n";
        cout << primea1.get_str(16) << "\n";
        cout << primea2.get_str(16) << "\n";

        cout << "\nPrime Pair 2:\n";
        cout << primeb1.get_str(16) << "\n";
        cout << primeb2.get_str(16) << "\n";


    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

