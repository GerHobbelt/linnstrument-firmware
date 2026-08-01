
#pragma once

// calculate the difference between now and a previous timestamp, taking a possible single overflow into account
template<typename T>
inline T calcTimeDelta(T now, T last) {
  if (now < last)
    return now + ~last;
  return now - last;
}
