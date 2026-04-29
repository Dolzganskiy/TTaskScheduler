#include "../include/forecast.h"

#include <iostream>
#include <string>
#include <expected>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

using json = nlohmann::json;

std::expected<MeteoInfo, std::string> Forecast::GetForecast(const std::pair<std::string, std::string>& pos) {
    std::cout << "Определение погоды..." << std::endl;
    const auto[lat, lon] = pos;

    cpr::Response r = cpr::Get(cpr::Url{"https://api.open-meteo.com/v1/forecast"}, 
        cpr::Parameters{{"latitude", lat}, {"longitude", lon}, 
        {"current", "temperature_2m,weather_code"}}, cpr::Timeout(15000));
    if (r.status_code != 200) {
        return std::unexpected("Open-Meteo HTTP" + std::to_string(r.status_code));
    }
    try {
        json parsed = json::parse(r.text);
        MeteoInfo info;
        info.temprature = parsed["current"]["temperature_2m"].get<int>();
        info.weather_code = parsed["current"]["weather_code"].get<int>();
        info.precipitation = InterpretPrecipitation(info.weather_code);
        return info;
    } catch (const std::exception& e) {
        return std::unexpected("JSON Parse Error: " + std::string(e.what()));
    }
}

std::string Forecast::InterpretPrecipitation(const int& weather_code) {
    if (weather_code == 0) return "Чистое небо";
    if (weather_code >= 1 && weather_code <= 3) return "Облачно";
    if (weather_code >= 45 && weather_code <= 48) return "Туман";
    if (weather_code >= 51 && weather_code <= 57) return "Морось";
    if (weather_code >= 61 && weather_code <= 67) return "Дождь";
    if (weather_code >= 71 && weather_code <= 73) return "Снег";
    if (weather_code == 75) return "Снегопад";
    if (weather_code == 77) return "Град";
    if (weather_code >= 80 && weather_code <= 82) return "Ливень";
    if (weather_code >= 85 && weather_code <= 86) return "Снегопад";
    if (weather_code >= 95 && weather_code <= 99) return "Гроза";
    return "Чистое небо";
}