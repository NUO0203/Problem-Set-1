/*
* Variant 2
* Straght Division
* Print at End
*/

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <chrono>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm> // For std::sort

 // --- Globals ---
std::mutex g_results_mutex;         // Mutex to protect the global results vector

struct PrimeResult {
    long long prime;
    int thread_num;
    std::string timestamp;

    bool operator<(const PrimeResult& other) const {
        return prime < other.prime;
    }
};

std::vector<PrimeResult> g_all_primes; // Shared vector to store all found prime results


std::map<std::string, long long> readConfig(const std::string& filename = "config.ini") {
    std::map<std::string, long long> config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << filename << ". Creating default." << std::endl;
        std::ofstream outfile(filename);
        outfile << "threads = 4" << std::endl;
        outfile << "max_number = 100000" << std::endl;
        outfile.close();
        config["threads"] = 4;
        config["max_number"] = 100000;
        return config;
    }
    std::string line;
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            try {
                config[key] = std::stoll(value);
            }
            catch (const std::exception& /*e*/) { // Unnamed variable to suppress warning
                std::cerr << "Warning: Could not parse line: " << line << std::endl;
            }
        }
    }
    file.close();
    if (config.find("threads") == config.end()) config["threads"] = 4;
    if (config.find("max_number") == config.end()) config["max_number"] = 100000;
    return config;
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;

    // Platform-specific fix for localtime (MSVC vs. others)
#ifdef _MSC_VER // Check if compiling with Microsoft Visual C++
    std::tm buf;
    localtime_s(&buf, &in_time_t);
    ss << std::put_time(&buf, "%Y-%m-%d %X");
#else
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
#endif

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

bool isPrime(long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i = i + 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}


void findPrimesInRange(long long start, long long end, int thread_num) {
    
    std::vector<PrimeResult> local_primes;

    for (long long n = start; n <= end; ++n) {
        if (isPrime(n)) {
            // Create a full result object and store it locally
            PrimeResult result;
            result.prime = n;
            result.thread_num = thread_num;
            result.timestamp = getCurrentTimestamp();
            local_primes.push_back(result);
        }
    }

    if (!local_primes.empty()) {
        std::lock_guard<std::mutex> lock(g_results_mutex);
        g_all_primes.insert(g_all_primes.end(), local_primes.begin(), local_primes.end());
    }
}


int main() {
    std::cout << "--- Variant 2 ---" << std::endl;
    std::cout << "--- Straight Division ---" << std::endl;
    std::cout << "--- Print at End ---" << std::endl;
    std::cout << "Run started at: " << getCurrentTimestamp() << std::endl;
    auto app_start_time = std::chrono::high_resolution_clock::now();

    auto config = readConfig();
    int thread_count = static_cast<int>(config["threads"]);
    long long max_number = config["max_number"];

    std::cout << "Configuration: " << thread_count << " threads | search up to " << max_number << "." << std::endl;
    std::cout << "Searching... (This may take a moment)" << std::endl;

    std::vector<std::thread> threads;
    long long range_per_thread = max_number / thread_count;

    // Launch threads
    for (int i = 0; i < thread_count; ++i) {
        long long start = i * range_per_thread + 1;
        if (i == 0) start = 2; // Start from 2

        long long end = (i == thread_count - 1)
            ? max_number
            : (i + 1) * range_per_thread;

        // Pass 'i + 1' as the new thread_num argument
        threads.emplace_back(findPrimesInRange, start, end, i + 1);
    }

    // Wait for all threads to complete
    for (auto& th : threads) {
        th.join();
    }
    std::cout << "All threads finished. Processing results..." << std::endl;

    // --- Print all results at the end ---

    // Sort the results for clean output
    std::sort(g_all_primes.begin(), g_all_primes.end());

    std::cout << "\nFound " << g_all_primes.size() << " prime numbers up to " << max_number << "." << std::endl;
    std::cout << "--- List of Primes (Sorted by Number) ---" << std::endl;
    // Print the full details for each result
    for (const auto& result : g_all_primes) {
        std::cout << "[Timestamp: " << result.timestamp
            << "] [Thread: " << result.thread_num
            << "] Found prime: " << result.prime << "\n";
    }
    std::cout << "--- End of List ---" << std::endl;


    auto app_end_time = std::chrono::high_resolution_clock::now();
    std::cout << "\nRun finished at: " << getCurrentTimestamp() << std::endl;

    std::chrono::duration<double> diff = app_end_time - app_start_time;
    std::cout << "Total execution time: " << diff.count() << " seconds" << std::endl;

    return 0;
}
