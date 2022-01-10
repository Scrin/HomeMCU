#include "mhz19.h"

void MHZ19::publishHomeassistant(int sensorState)
{
  std::map<String, String> attrs;
  attrs["sensor_state"] = String(sensorState);
  {
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = MHZ19::type,
        .field = "co2",
        .stateTopic = topic,
        .name = name,
        .fieldName = "CO2",
        .sensorName = "MH-Z19",
        .deviceClass = "carbon_dioxide",
        .unitOfMeasurement = "ppm",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = MHZ19::type,
        .field = "temperature",
        .stateTopic = topic,
        .name = name,
        .fieldName = "CO2 sensor temperature",
        .sensorName = "MH-Z19",
        .deviceClass = "temperature",
        .unitOfMeasurement = "ºC",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
}

void MHZ19::unpublishHomeassistant()
{
  HomeMCU::deleteDiscovery("sensor", MHZ19::type, "co2");
  HomeMCU::deleteDiscovery("sensor", MHZ19::type, "temperature");
}
