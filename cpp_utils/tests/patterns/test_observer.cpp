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

class TemperatureSensor : public patterns::Subject<TemperatureSensor> {
public:
    auto temperature() const -> Temperature { return val; }

    auto on_reading_changed(Temperature v) -> void
    {
        val = v;
        notify();
    }

private:
    Temperature val;
};

class HumiditySensor : public patterns::Subject<HumiditySensor> {
public:
    auto humidity() const -> Humidity { return val; }

    auto on_reading_changed(Humidity v) -> void
    {
        val = v;
        notify();
    }

private:
    Humidity val;
};

class Monitor {
public:
    virtual ~Monitor() = default;

    virtual auto display(const std::string& text) -> void = 0;
};

class WeatherStation {
public:
    WeatherStation(Monitor& temperature_display_,
                   Monitor& humidity_display_,
                   TemperatureSensor* temp_sensor_,
                   HumiditySensor* hum_sensor_)
        : temperature_display{temperature_display_}
        , humidity_display{humidity_display_}
        , temp_sensor_connection{temp_sensor_->attach(
              [&](TemperatureSensor* sensor) {
                  temp = sensor->temperature();
                  temperature_display.display(
                      std::format("Temperature: {}", temp.value));
              })}
        , hum_sensor_connection{
              hum_sensor_->attach([&](HumiditySensor* sensor) {
                  hum = sensor->humidity();
                  humidity_display.display(
                      std::format("Humidity: {}", hum.value));
              })}
    {
    }

private:
    Monitor& temperature_display;
    Monitor& humidity_display;
    TemperatureSensor::Connection temp_sensor_connection;
    HumiditySensor::Connection hum_sensor_connection;
    Temperature temp;
    Humidity hum;
};

class MockMonitor : public Monitor {
public:
    MOCK_METHOD(void, display, (const std::string&), (override));
};

class ObserverFixture : public ::testing::Test {
public:
    TemperatureSensor temp_sensor;
    HumiditySensor hum_sensor;
    MockMonitor temperature_display;
    MockMonitor humidity_display;
    WeatherStation station{
        temperature_display, humidity_display, &temp_sensor, &hum_sensor};
};

TEST_F(ObserverFixture, subject_notifies_observers)
{
    EXPECT_CALL(temperature_display, display("Temperature: 18"));
    EXPECT_CALL(humidity_display, display("Humidity: 40"));

    temp_sensor.on_reading_changed(Temperature{18});
    hum_sensor.on_reading_changed(Humidity{40});
}

TEST_F(ObserverFixture, handles_external_observer_removal_during_notification)
{
    MockMonitor monitor;
    auto victim_connection = temp_sensor.attach([&](TemperatureSensor* sensor) {
        monitor.display(std::format("{}", sensor->temperature().value));
    });
    auto removing_connection =
        temp_sensor.attach([&](TemperatureSensor* /* sensor */) {
            victim_connection.disconnect();
        });
    temp_sensor.on_reading_changed(Temperature{20});

    ASSERT_FALSE(victim_connection);
    EXPECT_CALL(monitor, display(_)).Times(0);

    temp_sensor.on_reading_changed(Temperature{15});
}

TEST_F(ObserverFixture, handles_self_disconnecting_connection)
{
    TemperatureSensor::Connection connection;
    connection = temp_sensor.attach(
        [&](TemperatureSensor* /* sensor */) { connection.disconnect(); });

    ASSERT_TRUE(connection);

    temp_sensor.on_reading_changed(Temperature{22});

    ASSERT_FALSE(connection);
}

TEST_F(ObserverFixture, handles_destruction_of_observer)
{
    MockMonitor hum_display;
    MockMonitor temp_display;
    auto temp_connection = temp_sensor.attach([&](TemperatureSensor* sensor) {
        temp_display.display(std::format("{}", sensor->temperature().value));
    });
    {
        auto hum_connection = hum_sensor.attach([&](HumiditySensor* sensor) {
            hum_display.display(std::format("{}", sensor->humidity().value));
        });
    }

    EXPECT_CALL(temp_display, display("20"));
    EXPECT_CALL(hum_display, display(_)).Times(0);

    temp_sensor.on_reading_changed(Temperature{20});
    hum_sensor.on_reading_changed(Humidity{50});
}
