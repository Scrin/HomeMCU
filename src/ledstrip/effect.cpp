#include "ledstrip.h"

const uint8_t colorwheel[360] = {0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 15, 17, 18, 20, 22, 24, 26, 28, 30, 32, 35, 37, 39, 42, 44, 47, 49, 52, 55, 58, 60, 63, 66, 69, 72, 75, 78, 81, 85, 88, 91, 94, 97, 101, 104, 107, 111, 114, 117, 121, 124, 127, 131, 134, 137, 141, 144, 147, 150, 154, 157, 160, 163, 167, 170, 173, 176, 179, 182, 185, 188, 191, 194, 197, 200, 202, 205, 208, 210, 213, 215, 217, 220, 222, 224, 226, 229, 231, 232, 234, 236, 238, 239, 241, 242, 244, 245, 246, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 246, 245, 244, 242, 241, 239, 238, 236, 234, 232, 231, 229, 226, 224, 222, 220, 217, 215, 213, 210, 208, 205, 202, 200, 197, 194, 191, 188, 185, 182, 179, 176, 173, 170, 167, 163, 160, 157, 154, 150, 147, 144, 141, 137, 134, 131, 127, 124, 121, 117, 114, 111, 107, 104, 101, 97, 94, 91, 88, 85, 81, 78, 75, 72, 69, 66, 63, 60, 58, 55, 52, 49, 47, 44, 42, 39, 37, 35, 32, 30, 28, 26, 24, 22, 20, 18, 17, 15, 13, 12, 11, 9, 8, 7, 6, 5, 4, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void Ledstrip::runEffect()
{
  switch (effect)
  {
  case eStable:
  case eGradient:
  case eCustom:
    break;
  case eSunrise:
    sunrise();
    dirty = true;
    break;
  case eColorLoop:
    for (int i = 0; i < ledCount; i++)
    {
      int angle = (Utils::uptime() / 20 + i) % 360;
      stripLeds[i] = RgbwColor(colorwheel[(angle + 120) % 360], colorwheel[angle], colorwheel[(angle + 240) % 360], 0);
    }
    dirty = true;
    break;
  }
}

void Ledstrip::effectStable()
{
  effect = eStable;
  for (int i = 0; i < ledCount; i++)
  {
    stripLeds[i] = RgbwColor(r, g, b, w);
  }
}

void Ledstrip::effectGradient()
{
  effect = eGradient;
  float x, stepSize = 1.f / (gradientExtent / 100.f) / ledCount;
  switch (gradientMode)
  {
  case gNear:
    x = 1.f;
    for (int i = 0; i < ledCount; i++)
    {
      x = max(x - stepSize, 0.f);
      stripLeds[i] = RgbwColor(r * x, g * x, b * x, w * x);
    }
    break;
  case gFar:
    x = 1.f;
    for (int i = ledCount - 1; i >= 0; i--)
    {
      x = max(x - stepSize, 0.f);
      stripLeds[i] = RgbwColor(r * x, g * x, b * x, w * x);
    }
    break;
    break;
  case gCenter:
    x = 1.f;
    for (int i = ledCount / 2; i < ledCount; i++)
    {
      x = max(x - stepSize, 0.f);
      stripLeds[i] = RgbwColor(r * x, g * x, b * x, w * x);
    }
    x = 1.f;
    for (int i = ledCount / 2 - 1; i >= 0; i--)
    {
      x = max(x - stepSize, 0.f);
      stripLeds[i] = RgbwColor(r * x, g * x, b * x, w * x);
    }
    break;
  case gEdges:
    x = 1.f;
    for (int i = 0; i < ledCount / 2; i++)
    {
      x = max(x - stepSize, 0.f);
      stripLeds[i] = RgbwColor(r * x, g * x, b * x, w * x);
    }
    x = 1.f;
    for (int i = ledCount - 1; i >= ledCount / 2; i--)
    {
      x = max(x - stepSize, 0.f);
      stripLeds[i] = RgbwColor(r * x, g * x, b * x, w * x);
    }
    break;
  }
}

void Ledstrip::effectCustom()
{
  effect = eCustom;
  for (int i = 0; i < ledCount; i++)
  {
    stripLeds[i] = customLeds[i];
  }
}

void Ledstrip::effectSunrise()
{
  effect = eSunrise;
  sunriseStart = Utils::uptime();
}

void Ledstrip::effectColorLoop()
{
  effect = eColorLoop;
  for (int i = 0; i < ledCount; i++)
  {
    int angle = (Utils::uptime() / 20 + i) % 360;
    stripLeds[i] = RgbwColor(colorwheel[(angle + 120) % 360], colorwheel[angle], colorwheel[(angle + 240) % 360], 0);
  }
}
