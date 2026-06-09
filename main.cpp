#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "spsc_queue.hpp"

// 1. Setup the test parameters
constexpr size_t QUEUE_CAPACITY = 1024; // Must be a power of 2!
constexpr int TOTAL_MESSAGES = 10'000'000;

// 2. Instantiate our hardware-optimized queue globally 
SPSCQueue<int, QUEUE_CAPACITY> queue;

// ---------------------------------------------------------
// PRODUCER THREAD
// ---------------------------------------------------------
void producer_thread() {
    for (int i = 1; i <= TOTAL_MESSAGES; ++i) {
        // Busy-wait (Spinlock): If the queue is full, keep hammering the CPU 
        // until the consumer frees up a slot. 
        while (!queue.push(i)) {
        }
    }
}

// ---------------------------------------------------------
// CONSUMER THREAD
// ---------------------------------------------------------
void consumer_thread() {
    int received_value = 0;
    for (int i = 1; i <= TOTAL_MESSAGES; ++i) {
        // Busy-wait: Keep checking until new data arrives
        while (!queue.pop(received_value)) {
            // Spin until data is ready
        }
        
        // Optional sanity check: Ensure we didn't drop or corrupt any packets
        if (received_value != i) {
            std::cerr << "FATAL ERROR: Data corruption detected!\n";
            std::exit(1);
        }
    }
}

// ---------------------------------------------------------
// MAIN BENCHMARK EXECUTION
// ---------------------------------------------------------
int main() {
    std::cout << "Starting Lock-Free SPSC Queue Benchmark...\n";
    std::cout << "Messages to process: " << TOTAL_MESSAGES << "\n";
    std::cout << "Queue Capacity: " << QUEUE_CAPACITY << "\n\n";

    // Start the high-resolution hardware timer
    auto start_time = std::chrono::high_resolution_clock::now();

    // Launch the threads on separate CPU cores
    std::thread producer(producer_thread);
    std::thread consumer(consumer_thread);

    // Wait for both threads to finish their 10 million operations
    producer.join();
    consumer.join();

    // Stop the timer
    auto end_time = std::chrono::high_resolution_clock::now();
    
    // Calculate performance metrics
    std::chrono::duration<double> diff = end_time - start_time;
    double seconds = diff.count();
    double msgs_per_second = TOTAL_MESSAGES / seconds;
    double latency_ns = (seconds * 1'000'000'000.0) / TOTAL_MESSAGES;

    // Print the results
    std::cout << "--- BENCHMARK RESULTS ---\n";
    std::cout << "Total Time:       " << seconds << " seconds\n";
    std::cout << "Throughput:       " << static_cast<long long>(msgs_per_second) << " messages/sec\n";
    std::cout << "Average Latency:  " << latency_ns << " nanoseconds/message\n";
    std::cout << "-------------------------\n";

    return 0;
}