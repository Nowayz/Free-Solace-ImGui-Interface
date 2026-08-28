#include "core/environment.h"

#include "core/product_info.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdlib>
#include <vector>

namespace solace::environment
{
namespace
{
std::string read_variable(const std::string& name)
{
#ifdef _WIN32
    const DWORD size = ::GetEnvironmentVariableA(name.c_str(), nullptr, 0);
    if (size == 0)
        return {};

    std::vector<char> buffer(size);
    const DWORD written =
        ::GetEnvironmentVariableA(name.c_str(), buffer.data(), static_cast<DWORD>(buffer.size()));
    return written > 0 && written < buffer.size() ? std::string(buffer.data(), written)
                                                  : std::string{};
#else
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : std::string{};
#endif
}

std::wstring read_variable(const std::wstring& name)
{
#ifdef _WIN32
    const DWORD size = ::GetEnvironmentVariableW(name.c_str(), nullptr, 0);
    if (size == 0)
        return {};

    std::vector<wchar_t> buffer(size);
    const DWORD written =
        ::GetEnvironmentVariableW(name.c_str(), buffer.data(), static_cast<DWORD>(buffer.size()));
    return written > 0 && written < buffer.size() ? std::wstring(buffer.data(), written)
                                                  : std::wstring{};
#else
    const std::string narrow(name.begin(), name.end());
    const char* value = std::getenv(narrow.c_str());
    return value ? std::wstring(value, value + std::char_traits<char>::length(value))
                 : std::wstring{};
#endif
}
} // namespace

std::string value(std::string_view key)
{
    return read_variable(product_info::environment_prefix + std::string(key));
}

std::wstring value(std::wstring_view key)
{
    return read_variable(product_info::environment_prefix_wide + std::wstring(key));
}
} // namespace solace::environment
