#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <iomanip>
#include "quantiprobe/SPSCQueue.hpp"
#include "quantiprobe/Timer.hpp"

using namespace quantiprobe;

struct ProfileEvent {
    uint32_t id;
    uint64_t duration_cycles; // 소요된 CPU 사이클
};

// 프로파일링 대상 함수 시뮬레이션
void simulate_work() {
    volatile int x = 0;
    for (int i = 0; i < 500; ++i) x++; // 약 수백 ns ~ 1us 정도의 연산
}

int main() {
    const size_t QUEUE_CAPACITY = 65536; // 큐 용량 증설
    const int TOTAL_OPERATIONS = 1'000'000;
    
    SPSCQueue<ProfileEvent> queue(QUEUE_CAPACITY);
    std::atomic<bool> producer_done{false};
    
    // 분석용 데이터 저장 (Consumer 스레드 전용)
    std::vector<uint64_t> latencies;
    latencies.reserve(TOTAL_OPERATIONS);
    uint64_t dropped_count = 0;

    // [Consumer Thread]
    auto consumer = std::thread([&]() {
        while (!producer_done || latencies.size() + dropped_count < TOTAL_OPERATIONS) {
            // benchmark_main.cpp 내부 consumer thread
            if (auto ev = queue.pop()) {
                latencies.push_back(ev->duration_cycles);
            } else {
                // ARM64용 대기 힌트
                asm volatile("yield" ::: "memory"); 
            }
        }
    });

    // [Producer Thread]
    auto start_bench = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TOTAL_OPERATIONS; ++i) {
        // 1. 측정 시작 사이클 기록
        uint64_t start = Timer::now();
        
        // 2. 실제 작업 수행
        simulate_work();
        
        // 3. 측정 종료 및 차이 계산
        uint64_t end = Timer::now();
        
        ProfileEvent ev{ static_cast<uint32_t>(i), end - start };
        
        // 4. SPSC 큐에 데이터 전송
        if (!queue.push(ev)) {
            dropped_count++;
        }
    }
    producer_done = true;
    consumer.join();

    auto end_bench = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_bench - start_bench;

    // 통계 계산
    uint64_t total_cycles = std::accumulate(latencies.begin(), latencies.end(), 0ULL);
    double avg_cycles = latencies.empty() ? 0 : (double)total_cycles / latencies.size();

    // 결과 출력
    std::cout << "========================================" << std::endl;
    std::cout << "    Quantiprobe Benchmark Results       " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Total Operations : " << TOTAL_OPERATIONS << std::endl;
    std::cout << "Successful Logs  : " << latencies.size() << std::endl;
    std::cout << "Dropped Logs     : " << dropped_count << " (" 
              << std::fixed << std::setprecision(2) 
              << (double)dropped_count / TOTAL_OPERATIONS * 100 << "%)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Avg Work Cycles  : " << avg_cycles << " cycles" << std::endl;
    std::cout << "Total Bench Time : " << diff.count() << " sec" << std::endl;
    std::cout << "Throughput       : " << (latencies.size() / diff.count()) / 1'000'000 
              << " M events/sec" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}