#ifndef QUANTIPROBE_SPSC_QUEUE_HPP
#define QUANTIPROBE_SPSC_QUEUE_HPP

#include <atomic>
#include <vector>
#include <optional>
#include <new>

template <typename T>
class SPSCQueue {
public:
    explicit SPSCQueue(size_t capacity) 
        : capacity_(capacity), buffer_(capacity + 1) {} // 인덱스 계산 편의를 위해 +1

    // Producer: 큐가 꽉 찼으면 즉시 false 반환 (Strict Mode)
    bool push(const T& data) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) % buffer_.size();

        // 큐가 가득 찼는지 확인 (Head가 Next Tail과 같다면 Full)
        // 이 읽기는 Consumer가 갱신한 head를 동기화해서 가져옴
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; // 데이터 유실을 감수하고 실행 흐름 보존
        }

        buffer_[current_tail] = data;
        
        // 데이터 쓰기가 완료된 후 tail을 업데이트함 (Release Barrier)
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    // Consumer: 데이터가 없으면 즉시 nullopt 반환
    std::optional<T> pop() {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        
        // 데이터가 있는지 확인 (Tail이 Head와 같다면 Empty)
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        T data = buffer_[current_head];
        
        // 데이터를 읽은 후 head를 업데이트함 (Release Barrier)
        head_.store((current_head + 1) % buffer_.size(), std::memory_order_release);
        return data;
    }

private:
    const size_t capacity_;
    std::vector<T> buffer_;

    // False Sharing 방지를 위한 캐시 라인 정렬
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
};

#endif