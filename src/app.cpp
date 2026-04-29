#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include "../include/app.h"
#include <iostream>

void App::Execute(const MeteoInfo& info, std::vector<std::vector<std::string>> places) {
    std::cout << "\n-------------------------------------------\n" << std::endl;
    
    std::cout << "Погода на улице: " << info.temprature << " градусов," << info.precipitation << std::endl;

    if (places.empty()) {
        std::cout << "К сожалению, интересных мест рядом не найдено." << std::endl;
        return;
    }

    std::cout << "Куда можно сходить:" << std::endl;
    int count = 0;
    for (const auto& place : places) {
        ++count;
        std::cout << count << ". Название: " << place[0] << std::endl;
        std::cout << "\tРасстояние: " << place[1] << std::endl;
        std::cout << "\tАдрес: " << place[2] << std::endl;
    }
    std::cout << "\n-------------------------------------------\n" << std::endl;
}