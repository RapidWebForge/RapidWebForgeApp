#ifndef TIMECHRONO_H
#define TIMECHRONO_H

#include <chrono>
#include <ctime>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

// Convert time_point to ISO 8601 string
inline std::string timePointToString(const std::chrono::system_clock::time_point &tp)
{
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = {};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &time); // Windows
#else
    localtime_r(&time, &tm); // POSIX
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

// Convert ISO 8601 string to time_point
inline std::chrono::system_clock::time_point stringToTimePoint(const std::string &timeStr)
{
    std::tm tm = {};
    std::istringstream iss(timeStr);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    std::time_t time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

#endif // TIMECHRONO_H
