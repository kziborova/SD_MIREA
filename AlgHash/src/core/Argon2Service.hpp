#pragma once

#include "hashService.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
#include <argon2.h>

class Argon2Service : public HashService {
public:
    long long timesec = 0;
    bool checkType(const std::string& hash) override;
    std::string hashing(const std::string& hash) override;

private:
    static constexpr int MAX_LENGTH = 6;
    static const std::string CHARSET;

    std::string cpuBruteforce(const std::string& targetHash);
    bool argon2Verify(const std::string& password, const std::string& hash);
};
