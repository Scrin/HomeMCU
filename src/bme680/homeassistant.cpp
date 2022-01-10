#include "bme680.h"

void BME680::publishHomeassistant()
{
  std::map<String, String> commonAttrs;
  commonAttrs["status"] = String(iaqSensor.status);
  commonAttrs["bme680_status"] = String(iaqSensor.bme680Status);
  commonAttrs["stab_status"] = String(iaqSensor.stabStatus);
  commonAttrs["run_in_status"] = String(iaqSensor.runInStatus);

  {
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = BME680::type,
        .field = "temperature",
        .stateTopic = topic,
        .name = name,
        .fieldName = "temperature",
        .sensorName = "BME680",
        .deviceClass = "temperature",
        .unitOfMeasurement = "ºC",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = commonAttrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = BME680::type,
        .field = "humidity",
        .stateTopic = topic,
        .name = name,
        .fieldName = "humidity",
        .sensorName = "BME680",
        .deviceClass = "humidity",
        .unitOfMeasurement = "%",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = commonAttrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = BME680::type,
        .field = "pressure",
        .stateTopic = topic,
        .name = name,
        .fieldName = "pressure",
        .sensorName = "BME680",
        .deviceClass = "pressure",
        .unitOfMeasurement = "hPa",
        .valueMutator = " / 100.0",
        .icon = nullptr,
        .attributes = commonAttrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    std::map<String, String> attrs(commonAttrs);
    attrs["iaq_accuracy"] = String(iaqSensor.iaqAccuracy);
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = BME680::type,
        .field = "iaq",
        .stateTopic = topic,
        .name = name,
        .fieldName = "indoor air quality",
        .sensorName = "BME680",
        .deviceClass = nullptr,
        .unitOfMeasurement = "IAQ",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    std::map<String, String> attrs(commonAttrs);
    attrs["static_iaq_accuracy"] = String(iaqSensor.staticIaqAccuracy);
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = BME680::type,
        .field = "staticIaq",
        .stateTopic = topic,
        .name = name,
        .fieldName = "static indoor air quality",
        .sensorName = "BME680",
        .deviceClass = nullptr,
        .unitOfMeasurement = "IAQ",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    std::map<String, String> attrs(commonAttrs);
    attrs["co2_equivalent_accuracy"] = String(iaqSensor.co2Accuracy);
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = BME680::type,
        .field = "co2Equivalent",
        .stateTopic = topic,
        .name = name,
        .fieldName = "CO2 equivalent",
        .sensorName = "BME680",
        .deviceClass = "carbon_dioxide",
        .unitOfMeasurement = "ppm",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    std::map<String, String> attrs(commonAttrs);
    attrs["bvoc_equivalent_accuracy"] = String(iaqSensor.breathVocAccuracy);
    HomeMCU::DiscoveryData data = {
        .component = "sensor",
        .type = BME680::type,
        .field = "breathVocEquivalent",
        .stateTopic = topic,
        .name = name,
        .fieldName = "bVOC",
        .sensorName = "BME680",
        .deviceClass = nullptr,
        .unitOfMeasurement = "ppm",
        .valueMutator = nullptr,
        .icon = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
}

void BME680::unpublishHomeassistant()
{
  HomeMCU::deleteDiscovery("sensor", BME680::type, "temperature");
  HomeMCU::deleteDiscovery("sensor", BME680::type, "humidity");
  HomeMCU::deleteDiscovery("sensor", BME680::type, "pressure");
  HomeMCU::deleteDiscovery("sensor", BME680::type, "iaq");
  HomeMCU::deleteDiscovery("sensor", BME680::type, "staticIaq");
  HomeMCU::deleteDiscovery("sensor", BME680::type, "co2Equivalent");
  HomeMCU::deleteDiscovery("sensor", BME680::type, "breathVocEquivalent");
}
