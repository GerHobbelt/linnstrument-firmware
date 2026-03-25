
#include "ls_FlashStorage.h"

// ----------------------------------------------------------------

#if 0

#include "./src/DueFlashStorage/src/DueFlashStorage.cpp"
#include "./src/DueFlashStorage/src/efc.cpp"
#include "./src/DueFlashStorage/src/flash_efc.cpp"

#endif


#if FLASH_DEBUG

void flash_debug(int level, const char *message) {
	static const char *levels[] = { "Info", "Warning", "Error" };
	DEBUGPRINT((0, levels[level]));
	DEBUGPRINT((0, ": "));
	DEBUGPRINT((0, message));
}

#endif
