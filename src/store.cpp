#include "store.hpp"
#include <fstream>

KVStore::KVStore()
{
    std::ifstream log(persistenceFile);

    if (!log)
    {
        return; // start with empty map
    }

    std::string command;

    while (std::getline(log, command))
    {
        if (command.substr(0, 3) == "SET")
        {
            // messy parsing
            std::string args = command.substr(3);
            size_t keyFirst = args.find_first_not_of(" \t\n\r");
            std::string temp = args.substr(keyFirst);
            size_t keyLast = temp.find_first_of(" \t\n\r");
            std::string key = temp.substr(0, keyLast);
            size_t valueFirst = keyLast + temp.substr(keyLast).find_first_not_of(" \t\n\r");
            std::string value = temp.substr(valueFirst);

            set(key, value, false);
        }
        else if (command.substr(0, 6) == "DELETE")
        {
            std::string args = command.substr(6);
            size_t first = args.find_first_not_of(" \t\n\r");
            remove(args.substr(first), false);
        }
    }
}

bool KVStore::set(const std::string &key, const std::string &value, bool updateLog)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (data.find(key) != data.end())
    {
        return false;
    }

    data[key] = value;

    if (updateLog)
    {
        std::ofstream log(persistenceFile, std::ios::app);
        log << "SET " << key << " " << value << "\n";
    }

    return true;
}
std::optional<std::string> KVStore::get(const std::string &key)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (data.find(key) != data.end())
    {
        return data[key];
    }

    return std::nullopt;
}
bool KVStore::remove(const std::string &key, bool updateLog)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (data.find(key) != data.end())
    {
        data.erase(key);

        if (updateLog)
        {
            std::ofstream log(persistenceFile, std::ios::app);
            log << "DELETE " << key << "\n";
        }

        return true;
    }

    return false;
}