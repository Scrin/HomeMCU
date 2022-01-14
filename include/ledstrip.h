#ifndef _ledstrip
#define _ledstrip

#include "common.h"

#include <NeoPixelBus.h>

class Ledstrip
{
public:
  static const char *type;
  bool enabled = false;

  void setup(JsonObject config);
  void loop();
  void stop();
  void command(const char *cmd);

private:
  enum Effect
  {
    eStable,
    eGradient,
    eCustom,
    eSunrise,
    eColorLoop
  };
  enum GradientMode
  {
    gEdges,
    gCenter,
    gNear,
    gFar
  };

  char *name;
  char topic[MQTT_MAX_TOPIC_LENGTH];
  char commandTopic[MQTT_MAX_TOPIC_LENGTH];

  NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> *strip;

  RgbwColor *stripLeds;
  RgbwColor *customLeds;

  uint16_t ledCount;
  uint64_t lastUpdate = 0;
  uint64_t transitionStart = 0;
  uint64_t transitionDuration = 0;
  bool dirty = true;
  bool stateDirty = true;

  bool on = false;
  bool inTransition = false;
  uint8_t r = 255;
  uint8_t g = 255;
  uint8_t b = 255;
  uint8_t w = 255;
  Effect effect = eStable;
  GradientMode gradientMode = gEdges;
  int gradientExtent = 50;
  uint64_t sunriseStart = 0;
  uint64_t sunriseDuration = 30 * 60 * 1000;

  // effect.cpp
  void runEffect();
  void effectStable();
  void effectGradient();
  void effectCustom();
  void effectSunrise();
  void effectColorLoop();

  // sunrise.cpp
  void sunrise();

  void publishHomeassistant();
};

#endif
