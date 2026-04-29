#pragma once

#include <cpr/cpr.h>
#include <string>
#include <expected>

class IPClientBase {
public:
    virtual std::expected<std::pair<std::string, std::string>, std::string> GetPosition() = 0;
    virtual ~IPClientBase() = default;
};

class Geoposition : public IPClientBase {
public:
    Geoposition(const std::string& key) : key_(key) {}

    std::expected<std::pair<std::string, std::string>, std::string> GetPosition() override;
private:
    std::string key_;
};