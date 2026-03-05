#include "Argon2Service.hpp"

const std::string Argon2Service::CHARSET =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*()-_=+[]{}|;:,.<>?";

bool Argon2Service::argon2Verify(const std::string& password, const std::string& hash) {
    int ret = argon2_verify(
        hash.c_str(),
        password.c_str(),
        password.size(),
        Argon2_id 
    );

    if (ret == ARGON2_OK) return true;

    ret = argon2_verify(hash.c_str(), password.c_str(), password.size(), Argon2_i);
    if (ret == ARGON2_OK) return true;

    ret = argon2_verify(hash.c_str(), password.c_str(), password.size(), Argon2_d);
    return ret == ARGON2_OK;
}

std::string Argon2Service::cpuBruteforce(const std::string& targetHash) {
    const std::string TAG = "[Argon2 " + targetHash.substr(0, 20) + "...]";
    const int csLen    = CHARSET.size();
    int numThreads     = std::thread::hardware_concurrency();

    std::cout << TAG << " CPU threads: " << numThreads << "\n";
    std::cout << TAG << " WARNING: Argon2 is memory-hard by design, "
              << "brute-force is extremely slow\n";

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

                    if (argon2Verify(candidate, targetHash)) {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        if (!found) {
                            found  = true;
                            result = candidate;
                            std::cout << TAG << " FOUND: " << candidate << "\n";
                        }
                    }

                    if (idx % 100 == 0 && idx > 0) {
                        double percent = (double)idx / total * 100.0;
                        std::cout << TAG << " Progress length=" << length
                                  << " : " << std::fixed << std::setprecision(2)
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

bool Argon2Service::checkType(const std::string& hash) {
    if (hash.size() < 30) return false;
    if (hash.substr(0, 8)  == "$argon2i$") return true;
    if (hash.substr(0, 9)  == "$argon2d$") return true;
    if (hash.substr(0, 10) == "$argon2id$") return true;
    return false;
}

std::string Argon2Service::hashing(const std::string& hash) {
    const std::string TAG = "[Argon2 " + hash.substr(0, 20) + "...]";

    std::cout << TAG << " Starting brute-force\n";
    std::cout << TAG << " Charset size: " << CHARSET.size()
              << ", max length: " << MAX_LENGTH << "\n";
    std::cout << TAG << " WARNING: Argon2 is ~10x slower than bcrypt (~"
              << std::thread::hardware_concurrency() * 2
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