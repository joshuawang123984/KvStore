#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <mutex>

class KVStore
{
private:
    std::unordered_map<std::string, std::string> data;
    std::mutex mutex;

public:
    bool set(const std::string &key, const std::string &value);
    std::optional<std::string> get(const std::string &key);
    bool remove(const std::string &key);
};