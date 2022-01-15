#include "ledstrip.h"

void Ledstrip::publishHomeassistant()
{
  std::map<String, String> attrs;
  attrs["led_count"] = ledCount;
  {
    HomeMCU::LightDiscoveryData data = {
        .type = Ledstrip::type,
        .field = "leds",
        .stateTopic = topic,
        .name = name,
        .fieldName = "Leds",
        .deviceClass = "light",
        .icon = nullptr,
        .commandTopic = commandTopic,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    std::vector<const char *> options;
    options.push_back("edges");
    options.push_back("center");
    options.push_back("near");
    options.push_back("far");
    HomeMCU::SelectDiscoveryData data = {
        .type = Ledstrip::type,
        .field = "gradientmode",
        .stateTopic = topic,
        .name = name,
        .fieldName = "Gradient mode",
        .icon = nullptr,
        .commandTopic = commandTopic,
        .commandTemplate = "{\"gradient_mode\":\"{{ value }}\"}",
        .valueTemplate = "{{ value_json.gradient_mode }}",
        .options = &options,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    HomeMCU::NumberDiscoveryData data = {
        .type = Ledstrip::type,
        .field = "gradientextent",
        .stateTopic = topic,
        .name = name,
        .fieldName = "Gradient extent",
        .min = 0,
        .max = 100,
        .step = 1,
        .unitOfMeasurement = "%",
        .icon = nullptr,
        .commandTopic = commandTopic,
        .commandTemplate = "{\"gradient_extent\":{{ value }}}",
        .valueTemplate = "{{ value_json.gradient_extent }}",
        .options = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
  {
    HomeMCU::NumberDiscoveryData data = {
        .type = Ledstrip::type,
        .field = "sunriseduration",
        .stateTopic = topic,
        .name = name,
        .fieldName = "Sunrise duration",
        .min = 60,
        .max = 3600,
        .step = 60,
        .unitOfMeasurement = "sec",
        .icon = nullptr,
        .commandTopic = commandTopic,
        .commandTemplate = "{\"sunrise_duration\":{{ value }}}",
        .valueTemplate = "{{ value_json.sunrise_duration }}",
        .options = nullptr,
        .attributes = attrs};
    HomeMCU::updateDiscovery(data);
  }
}
void Ledstrip::unpublishHomeassistant()
{
  HomeMCU::deleteDiscovery("light", Ledstrip::type, "leds");
  HomeMCU::deleteDiscovery("select", Ledstrip::type, "gradientmode");
  HomeMCU::deleteDiscovery("number", Ledstrip::type, "gradientextent");
  HomeMCU::deleteDiscovery("number", Ledstrip::type, "sunriseduration");
}
