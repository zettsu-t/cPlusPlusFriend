#include <cstdio>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <windows.h>

extern void checkPacket(void);

int main(int argc, char* argv[]) {
    size_t s = 0x1000;
    printf("%u\n",  s);
    printf("%lu\n", s);
    checkPacket();
    return 0;
}

// http://qiita.com/iwiwi/items/859753b829cddfd8426c
// を参考に作りました

struct IPv4HeaderPart {
    // リトルエンディアンでは順番に並べてもMSBから順には並ばない
    unsigned int version:4;
    unsigned int headerLength:4;
    unsigned int typeOfService:8;
    unsigned int totalLength:16;
    uint32_t others[5];
} __attribute__((__packed__));

union Packet {
public:
    constexpr Packet(const IPv4HeaderPart& argHeader) : header(argHeader) {}
    constexpr uint8_t operator()(size_t i) const { return octets[i]; }
    IPv4HeaderPart header;
    uint8_t        octets[1500];  // MTUを仮決め
} __attribute__((__packed__));

void checkPacket(void) {
    constexpr Packet packet({4, 3, 2, 1, {0, 0, 0, 0, 0}});

    static_assert(sizeof(packet.header) == 24, "not IPv4");
    // Cygwin-64 g++ 5.4.0ではコンパイルできない
#if defined(__GNUC__) && (__GNUC__ > 5)
    static_assert(packet.header.version == 4,       "");
    static_assert(packet.header.headerLength == 3,  "");
    static_assert(packet.header.typeOfService == 2, "");
    static_assert(packet.header.totalLength == 1,   "");
#endif
    static_assert(packet(1) == 0x40, "not IPv4");

    // リトルエンディアンでは順番に並べてもMSBから順には並ばないことを確認する
    for(size_t i=0; i<4; ++i) {
        unsigned int e = packet(i);
        std::cout << std::hex << std::setw(2) << std::setfill('0') << e;
    }

    std::cout << "\n";
    return;
}

struct YieldIsMacro {
    void Yield() { return; }
};

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
