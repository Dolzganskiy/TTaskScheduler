#include <iostream>
#include "../include/places.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Category {
    const std::string kPark = "4bf58dd8d48988d163941735";
    const std::string kBotanicPark = "52e81612bcbc57f1066b7a22";
    const std::string kMuseum = "4bf58dd8d48988d181941735";
    const std::string kRestaurant = "4d4b7105d754a06374d81259";
};

std::expected<std::vector<std::vector<std::string>>, std::string> Places::GetPlaces(
    const std::pair<std::string, std::string>& pos, 
    const MeteoInfo& info) {
    std::cout << "Поиск мест..." << std::endl;
    std::string category_id = ParseCategoryId(info.weather_code, info.temprature);

    const auto[lat, lon] = pos;
    cpr::Response r = cpr::Get(
        cpr::Url{"https://places-api.foursquare.com/places/search"},
        cpr::Header{
            {"X-Places-Api-Version", "2025-06-17"},
            {"accept", "application/json"},
            {"authorization", "Bearer " + key_}
        },
        cpr::Parameters{
            {"ll", lat + "," + lon},
            {"radius", "1500"},
            {"fsq_category_ids", category_id},
            {"fields", "categories,distance,location"},
            {"sort", "DISTANCE"},
            {"limit", "5"}
        }
    );

    if (r.status_code != 200) {
        return std::unexpected("Foursquare HTTP error: " + std::to_string(r.status_code));
    }

    try {
        json data = json::parse(r.text);
        std::vector<std::vector<std::string>> result;
        
        for (const auto& item : data["results"]) {
            std::vector<std::string> entertainment;

            std::string cat_name = "Unknown Category";
            if (!item["categories"].empty()) {
                cat_name = item["categories"][0]["name"].get<std::string>();
            }
            entertainment.push_back(cat_name);

            int dist = item["distance"].get<int>();
            entertainment.push_back(std::to_string(dist) + "m");

            std::string address = item["location"].value("formatted_address", "No address");
            entertainment.push_back(address);

            result.push_back(entertainment);
        }
        return result;
    } catch (...) {
        return std::unexpected("Foursquare JSON parse error");
    }
}

std::string Places::ParseCategoryId(int weather_code, int temperature) {
    Category categories;
    if (temperature >= 10 && weather_code >= 0 && weather_code <= 48) return categories.kPark;
    if (weather_code >= 0 && weather_code <= 50) return categories.kRestaurant;
    if (weather_code >= 51 && weather_code <= 67) return categories.kMuseum; 
    return categories.kBotanicPark;
}