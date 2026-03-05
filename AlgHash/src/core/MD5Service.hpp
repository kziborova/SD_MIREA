#pragma once
#define CL_TARGET_OPENCL_VERSION 300
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "hashService.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <cmath>

#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

class MD5Service : public HashService {
public:
    long long timesec = 0;
    bool checkType(const std::string& hash) override;
    std::string hashing(const std::string& hash) override;

private:
    static constexpr int MAX_LENGTH = 6;
    static const std::string CHARSET;

    std::string openclBruteforce(const std::string& targetHash);
    std::string uint8ToHex(const std::vector<uint8_t>& bytes);
};