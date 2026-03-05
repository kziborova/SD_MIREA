#pragma once

#include "createZIPService.h"

#include <fstream>
#include <array>

#include <QString>
#include "../../models/Auth.h"
#include "../../models/Files.h"
#include "../../models/Operations.h"

class STDCreateZIPService : public createZIPService {
    public:
    bool createZIP (const std::string& rawPath) override;
};