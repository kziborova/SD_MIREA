#pragma once
#define CL_TARGET_OPENCL_VERSION 300
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "hashService.hpp"
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <atomic>
#include <vector>
#include <cstring>
#include <cmath>

class SHA1Service : public HashService {
public:
    long long timesec = 0;
    bool checkType(const std::string& hash) override;
    std::string hashing(const std::string& hash) override;
private:
    std::string openclBruteforce(const std::string& targetHash);
    static constexpr int MAX_LENGTH = 6;
    static const std::string CHARSET;
    std::string uint8ToHex(const std::vector<uint8_t>& bytes);
};