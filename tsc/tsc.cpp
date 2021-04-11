#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <unistd.h>

#if (defined(_WIN32) | defined(__CYGWIN__))
#include <windows.h>
#else
#include <sched.h>
#endif

// Based on
// Intel 64 and IA-32 Architectures Software Developer's Manual

// Figure 3-6. Version Information Returned by CPUID in EAX
struct CpuVersion {
    unsigned int stepping:4;
    unsigned int model:4;
    unsigned int family:4;
    unsigned int processor_type:2;
    unsigned int reserved:2;
    unsigned int extended_model:4;
    unsigned int extended_family:8;
};

static_assert(sizeof(CpuVersion) == sizeof(uint32_t));

extern "C" {
    // It is not checked whether CPUID, RDTSC, and invariant TSC are available.
    extern void GetCpuVersion(CpuVersion* pResult);
    extern void GetTscRatio(uint32_t* pResult);
    extern uint64_t GetTsc(void);
}

class CpuFamilyModel {
public:
    CpuFamilyModel(void) {
        initCpuVersion();
        initRatio();
    }

    virtual ~CpuFamilyModel(void) = default;

    virtual uint64_t GetTscDenominator(void) const {
        return tsc_denominator_;
    }

    virtual uint64_t GetTscNumerator(void) const {
        return tsc_numerator_;
    }

    virtual uint64_t GetMhzDenominator(void) const {
        return mhz_denominator_;
    }

    virtual uint64_t GetMhzNumerator(void) const {
        return mhz_numerator_;
    }

private:
    void initCpuVersion(void) {
        GetCpuVersion(&version_);

        // CPUID Signature Values of DisplayFamily_DisplayModel
        uint32_t family_base = version_.family;
        if (family_base != 0x0f) {
            family_ = family_base;
        } else {
            family_ = version_.extended_family;
            family_ += family_base;
        }

        uint32_t model_base = version_.model;
        if ((family_base == 0x06) || (family_base == 0x0f)){
            model_ = version_.extended_model;
            model_ <<= 4;
            model_ += model_base;
        } else {
            model_ = model_base;
        }
    }

    void initRatio(void) {
        uint32_t tscRatio[2];
        GetTscRatio(tscRatio);
        tsc_denominator_ = tscRatio[0];
        tsc_numerator_ = tscRatio[1];

        // Table 18-85. Nominal Core Crystal Clock Frequency
        if ((family_ == 6) && (model_ == 0x55)) {
            mhz_denominator_ = 250;
        } else if ((family_ == 6) && (model_ == 0x5c)) {
            mhz_denominator_ = 192;
        } else {
            mhz_denominator_ = 240;
        }
    }

    CpuVersion version_;
    uint32_t family_ {0};
    uint32_t model_ {0};
    uint64_t tsc_denominator_ {0};
    uint64_t tsc_numerator_ {0};
    uint64_t mhz_denominator_ {0};
    uint64_t mhz_numerator_ {10};
};

struct TimeSet {
    uint64_t tsc {0};
    uint64_t diff {0};
};

TimeSet GetTimeDiff(const CpuFamilyModel& cpu, const TimeSet& previous, uint64_t unitInUsec) {
    TimeSet current;
    current.tsc = GetTsc();

    // Overflow is not checked here.
    uint64_t diff = current.tsc - previous.tsc;
    diff *= cpu.GetTscDenominator();
    diff *= cpu.GetMhzNumerator();
    diff *= unitInUsec;
    diff /= cpu.GetTscNumerator();
    diff /= cpu.GetMhzDenominator();
    current.diff = diff;
    return current;
}

int main(int argc, char* argv[]) {
    // Check bit fields
    CpuVersion dummyVersion {2};
    uint32_t bits = 0;
    static_assert(sizeof(bits) == sizeof(dummyVersion));
    memmove(&bits, &dummyVersion, sizeof(dummyVersion));
    assert(bits == 2);

    // It is not checked whether setting process affinity succeeds or fails.
#if (defined(_WIN32) | defined(__CYGWIN__))
    DWORD_PTR procMask = 1;
    SetProcessAffinityMask(GetCurrentProcess(), procMask);
#else
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif

    constexpr uint64_t unitInUsec = 100;
    constexpr size_t n_trials = 10;

    CpuFamilyModel cpu;
    std::vector<uint64_t> timer_vec;
    timer_vec.assign(n_trials, 0);
    TimeSet previous;

    bool first = true;
    for(size_t i = 0; i < n_trials;) {
        auto current = GetTimeDiff(cpu, previous, unitInUsec);
        if (!first) {
            timer_vec.at(i) = current.diff;
            ++i;
        }
        usleep(1000000);
        previous = current;
        first = false;
    }

    for(const auto& timer_value : timer_vec) {
        std::cout << timer_value << "\n";
    }

    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
