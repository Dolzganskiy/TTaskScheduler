#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <expected>

#include "../include/ip_client.h"
#include "../include/forecast.h"
#include "../include/places.h"
#include "../lib/TTaskScheduler.h"

using namespace testing;

using Coords = std::pair<std::string, std::string>;
using ForecastRes = std::expected<MeteoInfo, std::string>;
using PlacesRes = std::expected<std::vector<std::vector<std::string>>, std::string>;
using PositionRes = std::expected<Coords, std::string>;

class MockGeoposition : public IPClientBase {
public:
    MOCK_METHOD(PositionRes, GetPosition, (), (override));
};

class MockForecast : public IMeteoBase {
public:
    MOCK_METHOD(ForecastRes, GetForecast, ((const Coords&)), (override));
};

class MockPlaces : public IPlacesBase {
public:
    MOCK_METHOD(PlacesRes, GetPlaces, ((const Coords&), (const MeteoInfo&)), (override));
};

class MockIP : public IPClientBase {
public: 
    MOCK_METHOD((std::expected<std::pair<std::string, std::string>, std::string>), GetPosition, (), (override));
};

class MockMeteo : public IMeteoBase {
public:
    MOCK_METHOD((std::expected<MeteoInfo, std::string>), GetForecast, ((const std::pair<std::string, std::string>&)), (override));
};

TEST(AppIntegration, FullPipelineSuccess) {
    TTaskScheduler scheduler;
    
    MockGeoposition mockGeo;
    MockForecast mockWeather;
    MockPlaces mockPlaces;

    Coords coords = {"59.9", "30.3"};
    MeteoInfo info = {20, 0, "Clear"};
    std::vector<std::vector<std::string>> places = {{"Pub", "Nevsky"}};

    EXPECT_CALL(mockGeo, GetPosition()).WillOnce(Return(coords));
    EXPECT_CALL(mockWeather, GetForecast(coords)).WillOnce(Return(info));
    EXPECT_CALL(mockPlaces, GetPlaces(coords, info)).WillOnce(Return(places));

    auto t1 = scheduler.Add([&]() -> PositionRes { 
        return mockGeo.GetPosition(); 
    });
    
    auto t2 = scheduler.Add([&](auto res) -> ForecastRes {
        if (!res) return std::unexpected(res.error());
        return mockWeather.GetForecast(*res);
    }, t1.GetFutureResult<PositionRes>());

    auto t3 = scheduler.Add([&](auto p, auto w) -> PlacesRes {
        if (!p || !w) return std::unexpected("Dependency error");
        return mockPlaces.GetPlaces(*p, *w);
    }, t1.GetFutureResult<PositionRes>(), t2.GetFutureResult<ForecastRes>());

    auto t_final = scheduler.Add([&](auto p_res) -> bool {
        return p_res.has_value(); 
    }, t3.GetFutureResult<PlacesRes>());

    scheduler.ExecuteAll();

    bool is_success = t_final.GetResultSync(); 
    EXPECT_TRUE(is_success);
    
    auto final_data = t3.GetResultSync();
    ASSERT_TRUE(final_data.has_value());

    EXPECT_EQ((*final_data)[0][0], "Pub");

}

TEST(AppIntegration, ErrorPropagation) {
    TTaskScheduler scheduler;
    MockGeoposition mockGeo;

    EXPECT_CALL(mockGeo, GetPosition())
        .WillOnce(Return(std::unexpected("Network Error")));

    auto t1 = scheduler.Add([&]() -> PositionRes { 
        return mockGeo.GetPosition(); 
    });
    
    auto t2 = scheduler.Add([&](auto res) -> std::string {
        if (!res) return res.error();
        return "No Error";
    }, t1.GetFutureResult<PositionRes>());

    scheduler.ExecuteAll();

    std::string error_msg = t2.GetResultSync();
    EXPECT_EQ(error_msg, "Network Error");

}

TEST(SchedulerLogic, SimpleMath) {
    TTaskScheduler scheduler;

    auto t1 = scheduler.Add([]() -> int { return 10; });
    auto t2 = scheduler.Add([](int x) -> int { return x * 2; }, t1.GetFutureResult<int>());

    scheduler.ExecuteAll();

    int result = t2.GetResultSync();
    EXPECT_EQ(result, 20);
}


TEST(FullSystem, SuccessPath) {
    TTaskScheduler s;
    MockIP mockIp;
    MockMeteo mockMeteo;

    std::pair<std::string, std::string> coords = {"50", "30"};
    MeteoInfo info = {25, 0, "Sunny"};

    EXPECT_CALL(mockIp, GetPosition()).WillOnce(Return(coords));
    EXPECT_CALL(mockMeteo, GetForecast(coords)).WillOnce(Return(info));

    auto t1 = s.Add([&]() { return mockIp.GetPosition(); });
    auto t2 = s.Add([&](auto res) -> std::expected<MeteoInfo, std::string> {
        return res ? mockMeteo.GetForecast(*res) : std::unexpected(res.error());
    }, t1.GetFutureResult<std::expected<std::pair<std::string, std::string>, std::string>>());

    s.ExecuteAll();
    auto final_weather = t2.GetResultSync();
    ASSERT_TRUE(final_weather.has_value());
    EXPECT_EQ(final_weather->temprature, 25);
}

TEST(FullSystem, GeopositionFailure) {
    TTaskScheduler s;
    MockIP mockIp;
    EXPECT_CALL(mockIp, GetPosition()).WillOnce(Return(std::unexpected("No Internet")));

    auto t1 = s.Add([&]() { return mockIp.GetPosition(); });
    auto t2 = s.Add([&](auto res) -> int {
        return res ? 200 : 404;
    }, t1.GetFutureResult<std::expected<std::pair<std::string, std::string>, std::string>>());

    s.ExecuteAll();
    EXPECT_EQ(t2.GetResultSync(), 404);
}