#include "bcryptService.hpp"

const std::string bcryptService::CHARSET =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*()-_=+[]{}|;:,.<>?";

bool bcryptService::bcryptVerify(const std::string& password, const std::string& hash) {
    return BCrypt::validatePassword(password, hash);
}

std::string bcryptService::cpuBruteforce(const std::string& targetHash) {
    const std::string TAG = "[bcrypt " + targetHash.substr(0, 20) + "...]";
    const int csLen = CHARSET.size();
    int numThreads  = std::thread::hardware_concurrency();

    std::cout << TAG << " CPU threads: " << numThreads << "\n";

    std::atomic<bool> found(false);
    std::string result;
    std::mutex resultMutex;

    for (int length = 1; length <= MAX_LENGTH && !found; length++) {
        long long total     = (long long)std::pow(csLen, length);
        long long chunkSize = total / numThreads + 1;

        std::cout << TAG << " Trying length: " << length
                  << " (" << total << " combinations)\n";

        std::vector<std::thread> threads;

        for (int t = 0; t < numThreads; t++) {
            long long start = t * chunkSize;
            long long end   = std::min(start + chunkSize, total);

            threads.emplace_back([&, start, end, length]() {
                std::string candidate(length, CHARSET[0]);

                for (long long idx = start; idx < end && !found; idx++) {
                    long long tmp = idx;
                    for (int pos = length - 1; pos >= 0; pos--) {
                        candidate[pos] = CHARSET[tmp % csLen];
                        tmp /= csLen;
                    }

                    if (bcryptVerify(candidate, targetHash)) {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        if (!found) {
                            found  = true;
                            result = candidate;
                            std::cout << TAG << " FOUND: " << candidate << "\n";
                        }
                    }

                    if (idx % 1000 == 0 && idx > 0) {
                        double percent = (double)idx / total * 100.0;
                        std::cout << TAG << " Thread " << std::this_thread::get_id()
                                  << " progress: " << std::fixed << std::setprecision(2)
                                  << percent << "%\n";
                    }
                }
            });
        }

        for (auto& th : threads) th.join();

        if (result.empty() && !found)
            std::cout << TAG << " Length " << length << " exhausted\n";
    }

    return result;
}

bool bcryptService::checkType(const std::string& hash) {
    if (hash.size() != 60) return false;
    if (hash[0] != '$' || hash[1] != '2') return false;
    if (hash[2] != 'a' && hash[2] != 'b' && hash[2] != 'y') return false;
    if (hash[3] != '$') return false;
    return true;
}

std::string bcryptService::hashing(const std::string& hash) {
    const std::string TAG = "[bcrypt " + hash.substr(0, 20) + "...]";

    std::cout << TAG << " Starting brute-force\n";
    std::cout << TAG << " Charset size: " << CHARSET.size()
              << ", max length: " << MAX_LENGTH << "\n";
    std::cout << TAG << " WARNING: bcrypt is slow by design (~"
              << std::thread::hardware_concurrency() * 20
              << " hashes/sec on this CPU)\n";

    auto start = std::chrono::high_resolution_clock::now();

    std::string result = cpuBruteforce(hash);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    timesec  = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

    std::cout << TAG << " Time elapsed: " << ms << "ms (" << timesec << "s)\n";
    std::cout << TAG << " Result: " << (result.empty() ? "NOT FOUND" : result) << "\n";

    return result;
}