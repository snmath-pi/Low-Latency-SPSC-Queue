#include <atomic>
#include <array>
#include <cstddef>
template <typename T, size_t Capacity> 
class SPSCQueue {

    // PROTECTING THE OPTIMIZATION MATH HERE, works on power's of 2, if not fail the assert
    static_assert((Capacity != 0) && ((Capacity & (Capacity - 1)) == 0), 
                    "Capacity must be a power of 2!");

private:
    // 1. Flat memory layout here, zero pointer indirection, zero heap jumps (overpowers vectors)
    alignas(64) std::array<T, Capacity> buffer;

    // 2. Producer index: Aligned to its own cache line to prevent false sharing
    alignas(64) std::atomic<size_t> head{0};

    // 3. Consudmer index: Aligned to its own cache line
    alignas(64) std::atomic<size_t> tail{0};

public:
    SPSCQueue() = default;

    // ---------------------------------------------------------
    // PRODUCER HOT PATH (Called exclusively by Thread 1)
    // ---------------------------------------------------------

    bool push(const T& item) {
        // We only modify head, so reading it is relaxed
        size_t current_head = head.load(std::memory_order_relaxed);

        // 1-clock cycle bitwise wrap around (Replaces slow modulo %)
        size_t next_head = (current_head + 1) & (Capacity - 1);

        // Read the consumer's tail. We must ACQUIRE to ensure we see
        // the actual data state left by the Consumer thread.
        size_t current_tail = tail.load(std::memory_order_acquire);

        // Is the queue full?
        if (next_head == current_tail) {
            return false;
        }


        // Write data to our flat memory array
        buffer[current_head] = item;

        // Publish the new head to the consumer
        // RELEASE guarantees buffer is fully written before head updates.

        head.store(next_head, std::memory_order_release);

        return true;
    }

    // ---------------------------------------------------------
    // CONSUMER HOT PATH (Called exclusively by Thread 2)
    // ---------------------------------------------------------

    bool pop(T& item) {
        // We only modify tail, so reading it is relaxed
        size_t current_tail = tail.load(std::memory_order_relaxed);

        // Read the producer head, we must ACQUIRE to read published data
        size_t current_head = head.load(std::memory_order_acquire);

        // Is the queue empty?

        if (current_tail == current_head) {
            return false;
        }

        // Read the data from our flat memory array
        item = buffer[current_tail];

        // 1-clock cycle bitwise wrap around

        size_t next_tail = (current_tail  + 1) & (Capacity - 1);

        // Publish our next_tail index back to the producer.
        // RELEASE guarantees the buffer is fully read before the slot is freed.

        tail.store(next_tail, std::memory_order_release);

        return true;
    }
};