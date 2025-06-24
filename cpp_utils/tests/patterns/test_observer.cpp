#include "cpp_utils/patterns/Observer.h"
#include "gmock/gmock.h"
#include <format>

using ::testing::_;

struct Temperature {
    int value;

    friend auto operator==(const Temperature&, const Temperature&)
        -> bool = default;
};

struct Humidity {
    int value;

    friend auto operator==(const Humidity&, const Humidity&) -> bool = default;
};

class TemperatureSensor : public patterns::Subject {
public:
    auto temperature() const -> Temperature { return val; }

    auto on_reading_changed(Temperature v) -> void
    {
        val = v;
        notify_observers();
    }

private:
    Temperature val;
};

class HumiditySensor : public patterns::Subject {
public:
    auto humidity() const -> Humidity { return val; }

    auto on_reading_changed(Humidity v) -> void
    {
        val = v;
        notify_observers();
    }

private:
    Humidity val;
};

class Monitor {
public:
    virtual ~Monitor() = default;

    virtual auto display(const std::string& text) -> void = 0;
};

class WeatherStation : public patterns::Observer {
public:
    WeatherStation(Monitor& temperature_display_,
                   Monitor& humidity_display_,
                   TemperatureSensor* temp_sensor_,
                   HumiditySensor* hum_sensor_)
        : temperature_display{temperature_display_}
        , humidity_display{humidity_display_}
        , temp_sensor{temp_sensor_}
        , hum_sensor{hum_sensor_}
    {
        temp_sensor->attach(this);
        hum_sensor->attach(this);
    }

    ~WeatherStation()
    {
        temp_sensor->detach(this);
        hum_sensor->detach(this);
    }

    auto notify(patterns::Subject* subject) -> void override
    {
        if (subject == temp_sensor) {
            temp = temp_sensor->temperature();
            temperature_display.display(
                std::format("Temperature: {}", temp.value));
            return;
        }
        if (subject == hum_sensor) {
            hum = hum_sensor->humidity();
            humidity_display.display(std::format("Humidity: {}", hum.value));
            return;
        }
    }

private:
    Monitor& temperature_display;
    Monitor& humidity_display;
    TemperatureSensor* temp_sensor;
    HumiditySensor* hum_sensor;
    Temperature temp;
    Humidity hum;
};

class MockMonitor : public Monitor {
public:
    MOCK_METHOD(void, display, (const std::string&), (override));
};

class MockObserver : public patterns::Observer {
public:
    MOCK_METHOD(void, notify, (patterns::Subject*), (override));
};

class ObserverFixture : public ::testing::Test {
public:
    TemperatureSensor temp_sensor;
    HumiditySensor hum_sensor;
    MockMonitor temperature_display;
    MockMonitor humidity_display;
    WeatherStation sut{
        temperature_display, humidity_display, &temp_sensor, &hum_sensor};
};

TEST_F(ObserverFixture, subject_notifies_observers)
{
    EXPECT_CALL(temperature_display, display("Temperature: 18"));
    EXPECT_CALL(humidity_display, display(_)).Times(0);

    temp_sensor.on_reading_changed(Temperature{18});
}
