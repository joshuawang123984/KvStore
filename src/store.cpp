#include "store.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

void KVStore::compact()
{
    std::ofstream temp("data.tmp");

    if (!temp)
    {
        return;
    }

    for (const auto &[key, value] : data)
    {
        temp << "SET " << key << " " << value << "\n";
    }

    temp.close();

    // replace data.log with data.tmp
    std::filesystem::remove(persistenceFile);
    std::filesystem::rename("data.tmp", persistenceFile);
    operationCount = data.size();
}

KVStore::KVStore()
{
    std::ifstream log(persistenceFile);

    if (!log)
    {
        operationCount = 0;
        return; // start with empty map
    }

    std::string command;
    int line = 0;

    while (std::getline(log, command))
    {
        line++;
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
        else
        {
            std::cout << "Unknown Commnd at line: " << line << std::endl;
        }
    }

    operationCount = line;
}

bool KVStore::set(const std::string &key, const std::string &value, bool updateLog)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (data.find(key) != data.end())
    {
        return false;
    }

    if (updateLog)
    {
        std::ofstream log(persistenceFile, std::ios::app);
        if (!log)
        {
            return false;
        }

        log << "SET " << key << " " << value << "\n";
    }

    data[key] = value;
    if (updateLog)
    {
        operationCount++;

        if (operationCount >= constants::OPERATION_LIMIT)
        {
            compact();
        }
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
        if (updateLog)
        {
            std::ofstream log(persistenceFile, std::ios::app);
            if (!log)
            {
                return false;
            }

            log << "DELETE " << key << "\n";
        }

        data.erase(key);
        if (updateLog)
        {
            operationCount++;

            if (operationCount >= constants::OPERATION_LIMIT)
            {
                compact();
            }
        }

        return true;
    }

    return false;
}