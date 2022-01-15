#include "ledstrip.h"

void Ledstrip::command(const char *cmd)
{
  DynamicJsonDocument json(2048);
  DeserializationError error = deserializeJson(json, cmd);
  if (error)
  {
    Log::warn("Light command deserialization failed: " + String(error.c_str()));
    return;
  }

  const char *state = json["state"];
  if (state != nullptr)
  {
    bool changed = false;
    if (strcmp(state, "ON") == 0 && !on)
    {
      on = true;
      changed = true;
    }
    else if (strcmp(state, "OFF") == 0 && on)
    {
      on = false;
      changed = true;
    }
    if (changed)
    {
      if (json.containsKey("transition"))
      {
        uint64_t t = json["transition"];
        if (t > 0)
        {
          transitionDuration = t * 1000;
          inTransition = true;
        }
      }
      else
      {
        transitionDuration = 1000;
        inTransition = true;
      }
      transitionStart = Utils::uptime();
    }
  }

  JsonObject color = json["color"];
  if (!color.isNull())
  {
    r = color["r"];
    g = color["g"];
    b = color["b"];
    w = color["w"];
  }

  const char *newGradientMode = json["gradient_mode"];
  if (newGradientMode != nullptr)
  {
    if (strcmp(newGradientMode, "edges") == 0)
    {
      gradientMode = gEdges;
    }
    else if (strcmp(newGradientMode, "center") == 0)
    {
      gradientMode = gCenter;
    }
    else if (strcmp(newGradientMode, "near") == 0)
    {
      gradientMode = gNear;
    }
    else if (strcmp(newGradientMode, "far") == 0)
    {
      gradientMode = gFar;
    }
    else
    {
      Log::error("Unknown gradient mode: " + String(newGradientMode));
    }
    effectGradient();
  }

  if (json.containsKey("gradient_extent"))
  {
    gradientExtent = json["gradient_extent"];
    effectGradient();
  }

  const char *newEffect = json["effect"];
  if (newEffect != nullptr)
  {
    if (strcmp(newEffect, "stable") == 0)
    {
      effectStable();
    }
    else if (strcmp(newEffect, "gradient") == 0)
    {
      effectGradient();
    }
    else if (strcmp(newEffect, "custom") == 0)
    {
      effectCustom();
    }
    else if (strcmp(newEffect, "sunrise") == 0)
    {
      effectSunrise();
    }
    else if (strcmp(newEffect, "colorloop") == 0)
    {
      effectColorLoop();
    }
    else
    {
      Log::error("Unknown effect: " + String(newEffect));
    }
  }
  else if (!color.isNull()) // color was changed but effect not, certain effects vary based on the color
  {
    switch (effect)
    {
    case eStable:
      effectStable();
      break;
    case eGradient:
      effectGradient();
      break;
    case eCustom:
    case eSunrise:
    case eColorLoop:
      break;
    }
  }

  const char *custom = json["custom"];
  if (custom != nullptr)
  {
    for (int i = 0; i < ledCount; i++)
    {
      customLeds[i] = RgbwColor(0, 0, 0, 0);
    }
    for (int i = 0; custom[i] && i / 8 <= ledCount; i++)
    {
      int value;
      if (i % 2)
      {
        value = Utils::char2int(custom[i]);
      }
      else
      {
        value = Utils::char2int(custom[i]) << 4;
      }
      switch (i / 2 % 4)
      {
      case 0:
        customLeds[i / 8].R |= value;
        break;
      case 1:
        customLeds[i / 8].G |= value;
        break;
      case 2:
        customLeds[i / 8].B |= value;
        break;
      case 3:
        customLeds[i / 8].W |= value;
        break;
      }
      on = true;
      effectCustom();
    }
  }

  uint64_t sunrise = json["sunrise_duration"];
  if (sunrise > 0)
  {
    sunriseDuration = sunrise * 1000;
  }
  dirty = true;
  stateDirty = true;
}
