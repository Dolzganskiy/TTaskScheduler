#include <cpr/cpr.h>
#include <string>
#include <iostream>
#include "../include/ip_client.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::expected<std::pair<std::string, std::string>, std::string> Geoposition::GetPosition() {
    std::cout << "Определение местоположения..." << std::endl;
    cpr::Response r = cpr::Get(cpr::Url{"https://api.2ip.io?token=b883vxoormfec2np"},
        cpr::Parameters{{"token", key_}}, cpr::Timeout(15000));
    if (r.status_code != 200) {
        return std::unexpected<std::string>("2IP HTTP:" + std::to_string(r.status_code));
    }
    
    json parsed = json::parse(r.text);
    std::string lat = parsed["lat"].get<std::string>();
    std::string lon = parsed["lon"].get<std::string>();
    return std::pair<std::string, std::string>(lat, lon);
}