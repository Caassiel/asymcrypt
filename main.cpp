#include <iostream>
#include <cstdint>
#include <fstream>
#include <random>
using namespace std;

static const uint32_t SEQUENCE_LENGTH = 0xFFFFFF;

//1
class Default{
public:
    uint32_t next_bit(){
        static mt19937_64 gen(random_device{}());
        static uniform_int_distribution<uint32_t> dist(0, 10);
        return dist(gen) & 1u;
    }

};

//2
int BitLength(uint32_t x) {
    for (int i = 31; i >= 0; i--) {
        if (x != 0) {
            int bits = 0;
            while (x) { x >>= 1; bits++; }
            return bits;
        }
    }
    return 1;
}

class LehmerLow{

private:
    const uint64_t m = 0x10000000;
    const uint16_t c = 119;
    const uint32_t a = 0x10001;
    uint32_t x;

public:
    LehmerLow(uint32_t x_0){x = x_0;}

    uint32_t next_byte(){
        uint32_t output = (a*x + c) % m;
        x = output;
        return output & 0xFF;
    }

};

class LehmerHigh{

private:
    const uint64_t m = 0x10000000;
    const uint16_t c = 119;
    const uint32_t a = 0x10001;
    uint32_t x;

public:
    LehmerHigh(uint32_t x_0){x = x_0;}

    uint32_t next_byte(){
        uint32_t output = (a*x + c) % m;
        uint32_t s = BitLength(output);
        x = output;
        return (output >> (s - 8)) & 0xFF;
    }

};

//3, 4

class LFSR {
private:
    uint32_t state;
    int Size;
    vector<int> taps;
    uint32_t mask;

public:
    LFSR(uint32_t seed, int n, const vector<int>& t) {
        Size = n;
        taps = t;
        mask = (Size == 32) ? 0xFFFFFFFFu : ((1u << Size) - 1u);
        state = seed & mask;
        if (state == 0) state = 1;
    }

    inline uint32_t next_bit() {
        uint32_t output = state & 1u;
        uint32_t feedback = 0;

        for (int t : taps) feedback ^= (state >> t) & 1u;

        state >>= 1;
        state |= (feedback << (Size - 1));
        state &= mask;

        return output;
    }
};

class L20 {

private:
    LFSR L;

public:
    L20(uint32_t x):
        L(x, 20, {0,11,15,17})
    {}

    inline uint32_t next_bit() {
        return L.next_bit();
    }

};

class L89 {

private:
    LFSR L;

public:
    L89(uint32_t x):
        L(x, 89, {0,51})
    {}

    inline uint32_t next_bit() {
        return L.next_bit();
    }

};

//5

class Geffe_Generator {

private:
    LFSR L_1;
    LFSR L_2;
    LFSR L_3;

public:
    Geffe_Generator(uint32_t x, uint32_t y, uint32_t s):
        L_1(x, 11, {0,2}),
        L_2(y, 9, {0,1,3,4}),
        L_3(s, 10, {0,3})
    {}

    inline uint32_t next_bit() {

        uint32_t x = L_1.next_bit();
        uint32_t y = L_2.next_bit();
        uint32_t s = L_3.next_bit();

        return (x & s) ^ ((s ^ 1u) & y);
    }
};

//6

int get_bit (uint32_t x, int i){
    return (x & (1u << i)) & 1u;
}

void set_bit (uint32_t x, int i){
    if(i) x |= 1u << i;
    else x &= ~(1u << i);
}

uint32_t CyclicShiftLeft(uint32_t x, int s) {
    for (int i = 0; i < BitLength(x); i++)
        set_bit(x, get_bit(x, i + s));
    return x;
}

uint32_t CyclicShiftRight(uint32_t x, int s) {
    for (int i = 0; i < BitLength(x); i++)
        set_bit(x, get_bit(x, i - s));
    return x;
}


class Wolfram_Generator {

private:
    uint32_t r;

public:
    Wolfram_Generator(uint32_t r_0){r = r_0;}

    uint8_t next_bit() {
        uint8_t x = r & 1u;
        r = CyclicShiftLeft(r, 1) ^ (r | CyclicShiftRight(r, 1));
        return x;
    }

};



int main()
{
    ofstream file("output.txt");

    Wolfram_Generator gen1(5);

    for (uint32_t i = 0; i < SEQUENCE_LENGTH; i++){
        file << gen1.next_bit();
    }




    return 0;
}
