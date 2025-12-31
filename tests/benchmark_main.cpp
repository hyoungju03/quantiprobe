#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "quantiprobe/SPSCQueue.hpp" // 위에서 작성한 클래스

struct ProfileEvent {
    uint32_t id;
    uint64_t timestamp;
};

int main() {
    const size_t QUEUE_CAPACITY = 1024;
    const int TOTAL_OPERATIONS = 1'000'000; // 100만 개 데이터
    
    SPSCQueue<ProfileEvent> queue(QUEUE_CAPACITY);
    std::atomic<int> dropped_count{0};
    std::atomic<int> received_count{0};
    std::atomic<bool> producer_done{false};

    // [Consumer Thread] 데이터를 꺼내서 집계
    auto consumer = std::thread([&]() {
        while (!producer_done || received_count + dropped_count < TOTAL_OPERATIONS) {
            if (auto data = queue.pop()) {
                received_count++;
            } else {
                // 데이터가 없으면 잠시 쉬거나 busy-wait
                std::this_thread::yield(); 
            }
        }
    });

    // [Producer Thread] 데이터 생성 및 측정 시작
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TOTAL_OPERATIONS; ++i) {
        ProfileEvent ev{ static_cast<uint32_t>(i), 123456789ULL };
        
        // Strict Mode: 큐가 차있으면 기다리지 않고 버림
        if (!queue.push(ev)) {
            dropped_count++;
        }
    }
    producer_done = true;

    consumer.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    // 결과 출력
    std::cout << "--- Benchmark Results ---" << std::endl;
    std::cout << "Total Ops    : " << TOTAL_OPERATIONS << std::endl;
    std::cout << "Received     : " << received_count << std::endl;
    std::cout << "Dropped      : " << dropped_count << " (" 
              << (double)dropped_count / TOTAL_OPERATIONS * 100 << "%)" << std::endl;
    std::cout << "Time Taken   : " << diff.count() << " seconds" << std::endl;
    std::cout << "Throughput   : " << (received_count / diff.count()) / 1'000'000 << " M ops/sec" << std::endl;

    return 0;
}