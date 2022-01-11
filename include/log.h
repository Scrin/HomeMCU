#ifndef _log
#define _log

#include "common.h"

namespace Log
{
  void setup();
  void debug(String msg);
  void info(String msg);
  void warn(String msg);
  void error(String msg);
};

#endif
