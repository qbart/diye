#include "fmt.hpp"
#include <fmt/color.h>
#include <fmt/printf.h>

namespace fmtx
{
void Warn(const std::string &format) { fmt::println(fmt::format("{}[warn] {}{}", YELLOW, RESET, format)); }

void Error(const std::string &format) { fmt::println(fmt::format("{}[error] {}{}", RED, RESET, format)); }

void Info(const std::string &format) { fmt::println(fmt::format("{}[info] {}{}", BLUE, RESET, format)); }

void Debug(const std::string &format) { fmt::println(fmt::format("{}[debug] {}{}", CYAN, RESET, format)); }

void Success(const std::string &format) { fmt::println(fmt::format("{}[ok] {}{}", GREEN, RESET, format)); }
} // namespace fmtx