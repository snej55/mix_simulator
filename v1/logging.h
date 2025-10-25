#ifndef LOGGING_H
#define LOGGING_H

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

class Logger
{
public:
    explicit Logger(std::string_view dir, std::string_view name="")
     : m_dir{dir}, m_fileName{name}
    {
        std::stringstream fileName;
        // get current time
        std::time_t time {std::time(nullptr)};
        std::tm* now {std::localtime(&time)};
        // write date to stringstream
        fileName << now->tm_hour << ':' << (now->tm_min < 10 ? ("0" + std::to_string(now->tm_min)) : std::to_string(now->tm_min)) << ':' << (now->tm_sec < 10 ? ("0" + std::to_string(now->tm_sec)) : std::to_string(now->tm_sec))
         << "::" << now->tm_mday << '-' << now->tm_mon + 1 << '-' << now->tm_year + 1900 << ".log";
        m_fileName = fileName.str();
        // get file path
        m_path = (m_dir + '/' + m_fileName);
        // open log file
        m_file = std::ofstream{m_path.c_str()};
        if (m_file.is_open())
        {
            std::cout << "Started log file at `" << m_path << "`" << std::endl;
            // redirect cout to log file
            std::cout.rdbuf(m_file.rdbuf());
        } else {
            std::cout << "LOGGER::LOGGER::ERROR: Error starting log file at `" << m_path << "`" << std::endl;
        }
    }

    ~Logger()
    {
        close();
    }

    void close()
    {
        if (m_file.is_open())
        {
            m_file.close();
        }
    }

private:
    std::string m_dir;
    std::string m_fileName;
    std::string m_path;
    std::ofstream m_file;
};

#endif 