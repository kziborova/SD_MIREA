#pragma once

#include <string>
#include <vector>

class ExtractZIPService {
    public: // RAII must realization block and unblock file when ZIP edit
    virtual ~ExtractZIPService() = default;
    virtual void extractFile(const std::vector<std::string>& vec, const std::string& place) = 0;
    virtual bool checkBomb(const std::vector<std::string>& vec, const std::string& place) = 0;
};