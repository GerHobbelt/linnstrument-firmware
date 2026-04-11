
#ifndef LS_FLASHSTORAGE_H
#define LS_FLASHSTORAGE_H

#include "ls_debug.h"

#include "./libraries/DueFlashStorage/src/efc.h"
#include "./libraries/DueFlashStorage/src/flash_efc.h"
#include "./libraries/DueFlashStorage/src/DueFlashStorage.h"

#include "ls_alignToWord.h"


struct AddressInfo {
	byte* address;
	uint32_t size;
};


//  DueFlash is the main class for flash functions
class AppDataFlashStorage : protected DueFlashStorage {
protected:
	uint32_t activeBlockStartOffset;

  using super = DueFlashStorage;

public:
	AppDataFlashStorage();

	void factoryReset();

	byte readFirstTimeMarker() const;

	void clearFirstTimeMarker();

	void writePartialData(void *dataBufferOffset, const void* buff, uint32_t actual_size);

	const AddressInfo getProjectAddressInfo(uint16_t projectId) const;

	AddressInfo allocateProjectStorageSpace(uint16_t projectId, uint32_t size);

	const AddressInfo getSettingsAddressInfo() const;

	AddressInfo allocateSettingsStorageSpace(uint32_t size);

	const AddressInfo getConfigPresetAddressInfo(uint16_t presetId) const;

	AddressInfo allocateConfigPresetStorageSpace(uint16_t presetId, uint32_t size);

  void markSectionAsValid(const AddressInfo &chunkInfo);
  

#if 0

	// write() writes the specified amount of data into flash.
	// address is the offset from the flash start where the write should start
	// data is a pointer to the data to be written
	// dataLength is length of data in bytes

	byte read(uint32_t address) {
		return 253;
	}

	// This returns the physical address of the given flash offset. 0 returns the start of the flash (which is 0x80000 on the Due)
	const byte* readAddress(uint32_t address) {
		return nullptr;
	}

	// Return the flash offset for the given physical address.
	uint32_t getOffset(const byte* address) {
		return 0;
	}

	// This returns the physical address of the free flash memory after the program. It is retrieved from the linker map.
	// Writing to any address below the value returned by this function is likely going to corrupt the program memory and
	// then crash the CPU.
	static const byte* getFirstFreeBlock() {
		return nullptr;
	}

	static inline uint32_t getAvailableFlashSize() {
		return IFLASH0_SIZE + IFLASH1_SIZE - (getFirstFreeBlock() - FLASH_START);
	}

	// Test if the given offset is within the freely available space in the Flash.
	//
	// We DO NOT permit overwriting any application code or data, so the first available
	// address offset would be (getFirstFreeBlock() - FLASH_START).
	bool validateAddress(uint32_t address, uint32_t dataLength = 1) const {
		return false;
	}

	// These write methods write a byte or a block to the given offset.
	inline boolean write(uint32_t address, byte value) {
		//return write(address, &value, 1);
		return false;
	}
	inline boolean write(uint32_t address, const byte* data, uint32_t dataLength) {
		//return write(address, data, dataLength, true);
		return false;
	}

	boolean write(uint32_t address, const byte* data, uint32_t dataLength, bool with_locking) {
		return false;
	}

	// This writes directly to the given address. It must be in flash.
	inline boolean write(byte* address, const byte* data, uint32_t dataLength) {
		uint32_t offset = getOffset(address);
		return false;
		//return write(offset, data, dataLength);
	}

#endif

};

#endif
