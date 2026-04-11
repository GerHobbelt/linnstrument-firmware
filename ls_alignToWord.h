
#ifndef ALIGN2WORD_CONSTEXPR_H
#define ALIGN2WORD_CONSTEXPR_H

static constexpr inline int alignToWord32Boundary(int value) {
#if 0
  if (value % 4 == 0) {
    return value;
  }

  return ((value / 4) + 1) * 4;
#else
  return int((value + 3) / 4) * 4;
#endif
}

#endif
