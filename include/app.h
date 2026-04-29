#pragma once

#include <string>
#include <vector>
#include "forecast.h"

class App {
public:
    void Execute(const MeteoInfo& info, std::vector<std::vector<std::string>> places);
};