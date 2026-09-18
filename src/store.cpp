#include "store.hpp"

bool KVStore::set(const std::string &key, const std::string &value)
{
    if (data.find(key) != data.end())
    {
        return false;
    }

    data[key] = value;
    return true;
}
std::optional<std::string> KVStore::get(const std::string &key)
{
    if (data.find(key) != data.end())
    {
        return data[key];
    }

    return std::nullopt;
}
bool KVStore::remove(const std::string &key)
{
    if (data.find(key) != data.end())
    {
        data.erase(key);
        return true;
    }

    return false;
}