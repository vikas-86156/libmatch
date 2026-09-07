#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <iomanip>

// Mock interfaces matching your OrderBook core logic
enum class Side { BUY, SELL };
struct Order {
    int id;
    Side side;
    double price;
    int quantity;
    long long timestamp = 0;
};

// Simplified simulator for the harness demonstration
class MockOrderBook {
public:
    void addOrder(const Order& o) {
        // Volatile spin loop to simulate lightweight processing matching logic
        // without letting the compiler optimize the whole function away
        volatile int dummy = 0;
        for (int i = 0; i < 15; ++i) { dummy += i; }
    }
    void cancelOrder(int id) {
        volatile int dummy = 0;
        for (int i = 0; i < 8; ++i) { dummy += i; }
    }
    void matchOrders() {
        volatile int dummy = 0;
        for (int i = 0; i < 30; ++i) { dummy += i; }
    }
};

// Prints the percentile report cleanly
void reportMetrics(const std::string& operationName, std::vector<double>& latencies) {
    if (latencies.empty()) return;

    // 1. Sort latencies from fastest to slowest to calculate percentiles
    std::sort(latencies.begin(), latencies.end());

    size_t total = latencies.size();
    
    // 2. Extract exact index locations for p50, p99, and p99.9
    double p50  = latencies[static_cast<size_t>(total * 0.50)];
    double p99  = latencies[static_cast<size_t>(total * 0.99)];
    double p999 = latencies[static_cast<size_t>(total * 0.999)];
    double avg  = 0.0;
    for (double l : latencies) avg += l;
    avg /= total;

    std::cout << std::left << std::setw(10) << operationName 
              << " | Avg: " << std::setw(8) << std::fixed << std::setprecision(1) << avg << "ns"
              << " | p50: " << std::setw(8) << p50 << "ns"
              << " | p99: " << std::setw(8) << p99 << "ns"
              << " | p99.9: " << std::setw(8) << p999 << "ns\n";
}

int main() {
    constexpr size_t OPERATIONS_COUNT = 500000; // 500k requests synthetic load
    MockOrderBook orderBook;

    // Latency storage arrays (measured in nanoseconds)
    std::vector<double> addLatencies;
    std::vector<double> cancelLatencies;
    std::vector<double> matchLatencies;

    addLatencies.reserve(OPERATIONS_COUNT);
    cancelLatencies.reserve(OPERATIONS_COUNT);
    matchLatencies.reserve(OPERATIONS_COUNT);

    // Setup random generator for realistic random pricing loads
    std::mt19937 rng(42); 
    std::uniform_real_distribution<double> priceDist(99.0, 101.0);
    std::uniform_int_distribution<int> qtyDist(1, 100);

    std::cout << "🚀 Initializing Synthetic Load: Generating " << OPERATIONS_COUNT << " operations...\n";

    for (size_t i = 0; i < OPERATIONS_COUNT; ++i) {
        int orderId = static_cast<int>(i);
        Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
        double price = priceDist(rng);
        int qty = qtyDist(rng);

        // --- BENCHMARK: ADD OPERATION ---
        auto start = std::chrono::high_resolution_clock::now();
        orderBook.addOrder({orderId, side, price, qty, 0});
        auto end = std::chrono::high_resolution_clock::now();
        addLatencies.push_back(std::chrono::duration<double, std::nano>(end - start).count());

        // --- BENCHMARK: MATCH OPERATION ---
        start = std::chrono::high_resolution_clock::now();
        orderBook.matchOrders();
        end = std::chrono::high_resolution_clock::now();
        matchLatencies.push_back(std::chrono::duration<double, std::nano>(end - start).count());

        // --- BENCHMARK: CANCEL OPERATION (Every 5th order) ---
        if (i % 5 == 0) {
            start = std::chrono::high_resolution_clock::now();
            orderBook.cancelOrder(orderId);
            end = std::chrono::high_resolution_clock::now();
            cancelLatencies.push_back(std::chrono::duration<double, std::nano>(end - start).count());
        }
    }

    std::cout << "\n📊 --- LATENCY PERFORMANCES REPORT ---\n";
    reportMetrics("ADD", addLatencies);
    reportMetrics("MATCH", matchLatencies);
    reportMetrics("CANCEL", cancelLatencies);
    std::cout << "─────────────────────────────────────────────────────────────────\n";

    return 0;
}
