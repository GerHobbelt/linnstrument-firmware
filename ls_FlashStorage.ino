
#include "ls_FlashStorage.h"

// ----------------------------------------------------------------

#include "./libraries/DueFlashStorage/src/DueFlashStorage.cpp"
#include "./libraries/DueFlashStorage/src/efc.cpp"
#include "./libraries/DueFlashStorage/src/flash_efc.cpp"


#if FLASH_DEBUG

void flash_debug(int level, const char *message) {
	static const char *levels[] = { "Info", "Warning", "Error" };
	DEBUGPRINT((0, levels[level]));
	DEBUGPRINT((0, ": "));
	DEBUGPRINT((0, message));
}

#endif
