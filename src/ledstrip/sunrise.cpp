// Sunrise effect. Parts taken from https://github.com/thehookup/RGBW-Sunrise-Animation-Neopixel-/blob/master/Sunrise_CONFIGURE.ino

#include "ledstrip.h"

void Ledstrip::sunrise()
{
  uint64_t now = Utils::uptime();
  uint64_t timeSinceStart = now - sunriseStart;
  if (timeSinceStart > sunriseDuration)
  {
    return;
  }
  float progress = static_cast<float>(timeSinceStart) / static_cast<float>(sunriseDuration);

  int sunProgress = 256 * progress;
  int sunFadeStep = min(1.f, 2 * progress);

  int currentSun = map(sunProgress, 0, 256, 0, 100);
  if (currentSun % 2 != 0)
  {
    currentSun--;
  }

  int sunStart = (ledCount / 2) - (currentSun / 2);
  int newSunLeft = sunStart - 1;
  int newSunRight = sunStart + currentSun;
  int maxRed = max(1l, map(sunProgress, 0, 256, 0, 255));
  int maxGreen = max(0l, map(sunProgress, 0, 256, -192, 255));
  int maxBlue = max(0l, map(sunProgress, 0, 256, -255, 255));
  int maxWhite = max(0l, map(sunProgress, 0, 256, -32, 255));
  if (newSunLeft >= 0 && newSunRight <= ledCount && sunProgress > 0)
  {
    int redValue = sunFadeStep * maxRed;
    int greenValue = sunFadeStep * maxGreen;
    int blueValue = sunFadeStep * maxBlue;
    int whiteValue = sunFadeStep * maxWhite;
    stripLeds[newSunLeft] = RgbwColor(redValue, greenValue, blueValue, whiteValue);
    stripLeds[newSunRight] = RgbwColor(redValue, greenValue, blueValue, whiteValue);
  }

  for (int i = 0; i < newSunLeft; i++)
  {
    stripLeds[i] = RgbwColor(1, 0, 0, 0);
  }
  for (int i = newSunRight + 1; i <= ledCount; i++)
  {
    stripLeds[i] = RgbwColor(1, 0, 0, 0);
  }
  for (int i = sunStart; i < sunStart + currentSun; i++)
  {
    stripLeds[i] = RgbwColor(maxRed, maxGreen, maxBlue, maxWhite);
  }
}
