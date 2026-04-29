#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../lib/TTaskScheduler.h"
#include "../include/ip_client.h"
#include "../include/places.h"
#include "../include/forecast.h"
#include "../include/app.h"

int main() {
    TTaskScheduler scheduler;

    Geoposition ip("b883vxoormfec2np");
    Forecast weather;
    Places entertainments("DYX2YYBJTB1ID50FO3MJ3IRR3IRUTYPAOTYTZZZDVK3S32LF");
    App app;

    auto task_pos = scheduler.Add(&Geoposition::GetPosition, ip);

    auto task_pos_future = task_pos.GetFutureResult<std::expected<std::pair<std::string, std::string>, std::string>>();

    auto task_weather = scheduler.Add(
        [&](auto pos_expected) {
            if (!pos_expected) {
                return std::expected<MeteoInfo, std::string>(std::unexpected(pos_expected.error()));
            }
            return weather.GetForecast(*pos_expected); 
        }, 
        task_pos_future
    );

    auto task_weather_future = task_weather.GetFutureResult<std::expected<MeteoInfo, std::string>>();

    auto task_places = scheduler.Add(
        [&](auto pos_expected, auto weather_expected) {
            if (!pos_expected) {
                return std::expected<std::vector<std::vector<std::string>>, std::string>(std::unexpected(pos_expected.error()));
            };
            if (!weather_expected) {
                return std::expected<std::vector<std::vector<std::string>>, std::string>(std::unexpected(weather_expected.error()));
            }
            return entertainments.GetPlaces(*pos_expected, *weather_expected);
        },
        task_pos_future, 
        task_weather_future
    );

    auto task_places_future = task_places.GetFutureResult<std::expected<std::vector<std::vector<std::string>>, std::string>>();

    auto task_execute = scheduler.Add(
        [&](auto weather_expected, auto places_expected) {
            if (!weather_expected) {
                std::cerr << "Ошибка распознования погоды: " << weather_expected.error() << std::endl;
                return;
            }
            if (!places_expected) {
                std::cerr << "Ошибка поиска мест: " << places_expected.error() << std::endl;
                return;
            }
            app.Execute(*weather_expected, *places_expected);
        },
        task_weather_future, 
        task_places_future
    );

    scheduler.ExecuteAll();
}