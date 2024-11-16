#pragma once

#include <fmt/core.h>
#include <fmt/format.h>
#include <string>

namespace fmtx
{
    static auto RESET = "\033[0m";
    static auto RED = "\033[0;31m";
    static auto YELLOW = "\033[0;33m";
    static auto BLUE = "\033[0;34m";
    static auto GREEN = "\033[0;32m";
    static auto CYAN = "\033[0;36m";

    void Warn(const std::string &format);
    void Error(const std::string &format);
    void Info(const std::string &format);
    void Debug(const std::string &format);
    void Success(const std::string &format);
}