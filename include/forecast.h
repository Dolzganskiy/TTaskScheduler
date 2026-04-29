#pragma once

#include <string>
#include <expected>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct MeteoInfo {
    int temprature;
    int weather_code;
    std::string precipitation;

    bool operator==(const MeteoInfo& other) const = default;
};

class IMeteoBase {
public:
    virtual std::expected<MeteoInfo, std::string> GetForecast(const std::pair<std::string, std::string>& pos) = 0;
    virtual ~IMeteoBase() = default;
};

class Forecast : public IMeteoBase {
public:
    std::expected<MeteoInfo, std::string> GetForecast(const std::pair<std::string, std::string>& pos) override;
private:
    MeteoInfo info_;
    std::string InterpretPrecipitation(const int& weather_code);
};