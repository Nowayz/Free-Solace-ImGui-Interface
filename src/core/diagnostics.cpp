#include "core/diagnostics.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace solace::diagnostics
{
namespace
{
#ifdef _WIN32
constexpr std::uintmax_t maximum_log_size = 1024 * 1024;

std::filesystem::path local_app_data()
{
#ifdef _WIN32
    const DWORD size = ::GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0);
    if (size > 0)
    {
        std::vector<wchar_t> buffer(size);
        const DWORD written = ::GetEnvironmentVariableW(L"LOCALAPPDATA", buffer.data(),
                                                        static_cast<DWORD>(buffer.size()));
        if (written > 0 && written < buffer.size())
            return std::filesystem::path(buffer.data());
    }
#endif

    std::error_code error;
    return std::filesystem::temp_directory_path(error);
}

std::filesystem::path log_path()
{
    static const std::filesystem::path path = []
    {
        std::filesystem::path directory = local_app_data() / L"Solace" / L"logs";
        std::error_code error;
        std::filesystem::create_directories(directory, error);

        std::filesystem::path current = directory / L"solace.log";
        if (std::filesystem::file_size(current, error) > maximum_log_size && !error)
        {
            const std::filesystem::path previous = directory / L"solace.log.1";
            std::filesystem::remove(previous, error);
            error.clear();
            std::filesystem::rename(current, previous, error);
        }
        return current;
    }();
    return path;
}
#endif

void write(const char* level, std::string_view subsystem, std::string_view message,
           long system_code) noexcept
{
    try
    {
#ifdef _WIN32
        SYSTEMTIME time{};
        ::GetLocalTime(&time);
#else
        const std::time_t raw = std::time(nullptr);
        std::tm time{};
        localtime_r(&raw, &time);
#endif

        std::ostringstream line;
#ifdef _WIN32
        line << std::setfill('0') << time.wYear << '-' << std::setw(2) << time.wMonth << '-'
             << std::setw(2) << time.wDay << ' ' << std::setw(2) << time.wHour << ':'
             << std::setw(2) << time.wMinute << ':' << std::setw(2) << time.wSecond << " [" << level
             << "] [" << subsystem << "] " << message;
#else
        line << std::put_time(&time, "%Y-%m-%d %H:%M:%S") << " [" << level << "] [" << subsystem
             << "] " << message;
#endif
        if (system_code != 0)
            line << " (0x" << std::hex << std::uppercase << static_cast<unsigned long>(system_code)
                 << ')';
        line << '\n';

        const std::string text = line.str();
#ifdef _WIN32
        ::OutputDebugStringA(text.c_str());

        static std::mutex mutex;
        const std::lock_guard lock(mutex);
        std::ofstream output(log_path(), std::ios::app | std::ios::binary);
        output.write(text.data(), static_cast<std::streamsize>(text.size()));
#else
        std::clog << text;
#endif
    }
    catch (...)
    {
#ifdef _WIN32
        ::OutputDebugStringA("[Solace] diagnostics write failed.\n");
#else
        std::clog << "[Solace] diagnostics write failed.\n";
#endif
    }
}
} // namespace

void info(std::string_view subsystem, std::string_view message) noexcept
{
    write("info", subsystem, message, 0);
}

void warning(std::string_view subsystem, std::string_view message, long system_code) noexcept
{
    write("warning", subsystem, message, system_code);
}

void error(std::string_view subsystem, std::string_view message, long system_code) noexcept
{
    write("error", subsystem, message, system_code);
}
} // namespace solace::diagnostics
