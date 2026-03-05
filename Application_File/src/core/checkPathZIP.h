#pragma once

#include <array>
#include <string_view>

struct PathValidationResZIP {
    bool empty = true;
    bool exists = true;
    bool isFile = true;
    bool readable = true;
    bool writable = true;
    bool removable = true;
    bool hasExtension = true;
    bool canonicalizable = true;
    bool insideWorkingDir = true;

    bool zipExtension = true;
    bool zipSignature = true;
    bool zipReadable = true;
    bool zipNotCorrupted = true;

    bool dirExecutable = true;
    bool userReadable = true;
    bool dirCreatable = true;
    bool notAboveAppDir = true;
    bool hasTraversal = true;
    bool noDuplicate = true;

    auto get_fields() const {
        return std::array<std::pair<std::string_view, const bool*>, 19>{{
            {"empty", &empty},
            {"exists", &exists},
            {"isFile", &isFile},
            {"readable", &readable},
            {"writable", &writable},
            {"removable", &removable},
            {"hasExtension", &hasExtension},
            {"canonicalizable", &canonicalizable},
            {"insideWorkingDir", &insideWorkingDir},
            {"zipExtension", &zipExtension},
            {"zipSignature", &zipSignature},
            {"zipReadable", &zipReadable},
            {"zipNotCorrupted", &zipNotCorrupted},
            {"dirExecutable", &dirExecutable},
            {"userReadable", &userReadable},
            {"dirCreatable", &dirCreatable},
            {"notAboveAppDir", &notAboveAppDir},
            {"hasTraversal", &hasTraversal},
            {"noDuplicate", &noDuplicate}
        }};
    }
};

enum class PathAccessModeZIP {
    CONTENT,
    EDIT,
    COMPRESS,
    EXTRACT,
    EXTRACTPATH
};