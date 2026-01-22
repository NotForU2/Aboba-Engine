#pragma once
#include <iostream>
#include <string>

enum LogLevel
{
  Info,
  Warning,
  Error
};

class Logger
{
public:
  static void Log(LogLevel level, const std::string &message)
  {
    switch (level)
    {
    case LogLevel::Info:
      std::cout << "[INFO]: " << message << std::endl;
      break;
    case LogLevel::Warning:
      std::cout << "[WARN]: " << message << std::endl;
      break;
    case LogLevel::Error:
      std::cout << "[ERR ]: " << message << std::endl;
      break;
    default:
      break;
    }
  }
};