#ifndef KARM_UTILITY_H
#define KARM_UTILITY_H

#include "karmutility.h"

QString formatTime( long minutes )
{
  QString time;
  time.sprintf("%ld:%02ld", minutes / 60, labs(minutes % 60));
  return time;
}

#endif // KARM_UTILITY_H
