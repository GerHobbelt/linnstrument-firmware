
#ifndef RAMFUNC_ATTR_H
#define RAMFUNC_ATTR_H

#ifdef __ICCARM__
#define RAMFUNC __ramfunc
#else
#define RAMFUNC __attribute__ ((section(".ramfunc")))
#endif

#endif
