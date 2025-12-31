#ifndef QUANTIPROBE_TIMER_HPP
#define QUANTIPROBE_TIMER_HPP

#include <cstdint>

namespace quantiprobe {

class Timer {
public:
    static inline uint64_t now() {
        uint64_t vct;
        // ARM64에서 시스템 카운터를 읽는 어셈블리
        asm volatile("mrs %0, cntvct_el0" : "=r"(vct));
        return vct;
    }

    // ARM64 카운터의 주파수(Hz)를 읽어오는 함수 (나노초 변환용)
    static inline uint64_t get_frequency() {
        uint64_t freq;
        asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
        return freq;
    }
};

} // namespace quantiprobe

#endif