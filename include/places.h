#pragma once

#include <expected>
#include <vector>
#include <string>
#include "forecast.h"

class IPlacesBase {
public:
    virtual std::expected<std::vector<std::vector<std::string>>, std::string> GetPlaces(
        const std::pair<std::string, std::string>& pos, 
        const MeteoInfo& info) = 0;
    virtual ~IPlacesBase() = default;
};

class Places : public IPlacesBase {
public:
    Places(const std::string& key) : key_(key) {}

    std::expected<std::vector<std::vector<std::string>>, std::string> GetPlaces(
        const std::pair<std::string, std::string>& pos, 
        const MeteoInfo& info) override;
private:
    std::string key_;
    std::string ParseCategoryId(int weather_code, int temperature);
};