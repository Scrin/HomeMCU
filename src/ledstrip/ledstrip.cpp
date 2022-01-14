#include "ledstrip.h"

const char *Ledstrip::type = "ledstrip";

void Ledstrip::setup(JsonObject config)
{
  if (!config["enabled"])
  {
    publishHomeassistant();
    Log::info("ledstrip disabled");
    return;
  }
  Log::info("ledstrip enabled");
  const char *name = config["name"];
  if (name != nullptr)
  {
    this->name = strdup(name);
  }
  else
  {
    this->name = HomeMCU::name;
  }

  ledCount = config["led_count"];
  if (ledCount == 0)
  {
    Log::warn("led_count not set in config, defaulting to 100 leds");
    ledCount = 100;
  }

  Utils::getStateTopic(this->topic, Ledstrip::type);
  Utils::getCommandTopic(this->commandTopic, Ledstrip::type);

  strip = new NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod>(ledCount);
  stripLeds = new RgbwColor[ledCount];
  customLeds = new RgbwColor[ledCount];

  for (int i = 0; i < ledCount; i++)
  {
    stripLeds[i] = RgbwColor(r, g, b, w);
    customLeds[i] = RgbwColor(0, 0, 0, 0);
  }

  strip->Begin();
  strip->Show();
  enabled = true;
}

void Ledstrip::loop()
{
  if (on || inTransition)
    runEffect();

  float multipler = 1.0f;
  if (inTransition)
  {
    uint64_t now = Utils::uptime();
    uint64_t timeSinceStart = now - transitionStart;
    if (timeSinceStart > transitionDuration)
    {
      inTransition = false;
    }
    else
    {
      multipler = static_cast<float>(timeSinceStart) / static_cast<float>(transitionDuration);
      if (!on)
      {
        multipler = 1.f - multipler;
      }
    }
    dirty = true;
  }

  if (dirty)
  {
    if (on || inTransition)
    {
      for (int i = 0; i < ledCount; i++)
      {
        strip->SetPixelColor(i, RgbwColor(stripLeds[i].R * multipler, stripLeds[i].G * multipler, stripLeds[i].B * multipler, stripLeds[i].W * multipler));
      }
    }
    else
    {
      for (int i = 0; i < ledCount; i++)
      {
        strip->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
      }
    }
    strip->Show();
    dirty = false;
  }

  uint64_t now = Utils::uptime();
  if (lastUpdate + 10000 > now && !stateDirty)
    return;

  lastUpdate = now;

  DynamicJsonDocument json(1024);
  json["state"] = on ? "ON" : "OFF";
  json["color_mode"] = "rgbw";
  json["color"]["r"] = r;
  json["color"]["g"] = g;
  json["color"]["b"] = b;
  json["color"]["w"] = w;

  switch (effect)
  {
  case eStable:
    json["effect"] = "stable";
    break;
  case eGradient:
    json["effect"] = "gradient";
    break;
  case eCustom:
    json["effect"] = "custom";
    break;
  case eSunrise:
    json["effect"] = "sunrise";
    break;
  case eColorLoop:
    json["effect"] = "colorloop";
    break;
  }

  switch (gradientMode)
  {
  case gEdges:
    json["gradient_mode"] = "edges";
    break;
  case gCenter:
    json["gradient_mode"] = "center";
    break;
  case gNear:
    json["gradient_mode"] = "near";
    break;
  case gFar:
    json["gradient_mode"] = "far";
    break;
  }

  json["gradient_extent"] = gradientExtent;
  json["sunrise_duration"] = sunriseDuration / (60 * 1000);

  String msg;
  serializeJson(json, msg);
  HomeMCU::client.publish(topic, msg.c_str());
  publishHomeassistant();
  stateDirty = false;
}

void Ledstrip::stop()
{
  for (int i = 0; i < ledCount; i++)
  {
    strip->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
  }
  strip->Show();
  while (!strip->CanShow())
    yield();
  delete strip;
}
