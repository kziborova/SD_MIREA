#pragma once

#include <string>

class HashService {
    public:
    virtual ~HashService() = default;
    virtual bool checkType(const std::string& hash) = 0;
    virtual std::string hashing(const std::string& hash)   = 0;
};