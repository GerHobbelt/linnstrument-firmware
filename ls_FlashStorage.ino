
#include "ls_FlashStorage.h"

// ----------------------------------------------------------------

#if 0

#include "./src/DueFlashStorage/src/DueFlashStorage.cpp"
#include "./src/DueFlashStorage/src/efc.cpp"
#include "./src/DueFlashStorage/src/flash_efc.cpp"

#endif


#if FLASH_DEBUG

void flash_debug(int level, const char* message) {
	static const char* levels[] = { "Info", "Warning", "Error" };
	DEBUGPRINT((0, levels[level]));
	DEBUGPRINT((0, ": "));
	DEBUGPRINT((0, message));
}

#endif

#if 0

#define IFLASH0_SIZE (0x40000u)
#define IFLASH0_PAGE_SIZE (256u)
#define IFLASH0_LOCK_REGION_SIZE (16384u)
#define IFLASH0_NB_OF_PAGES (1024u)
#define IFLASH1_SIZE (0x40000u)
#define IFLASH1_PAGE_SIZE (256u)
#define IFLASH1_LOCK_REGION_SIZE (16384u)
#define IFLASH1_NB_OF_PAGES (1024u)
#define IRAM0_SIZE (0x10000u)
#define IRAM1_SIZE (0x8000u)
#define NFCRAM_SIZE (0x1000u)
#define IFLASH_SIZE (IFLASH0_SIZE + IFLASH1_SIZE)
#define IRAM_SIZE (IRAM0_SIZE + IRAM1_SIZE)

byte marker = dueFlashStorage.read(PROJECTS_OFFSET);

constexpr const int PROJECTS_OFFSET = 4;
constexpr const int PROJECT_VERSION_MARKER_SIZE = 4;
constexpr const int PROJECT_INDEXES_COUNT = 20;
constexpr const int PROJECTS_MARKERS_SIZE = alignToWord32Boundary(PROJECT_VERSION_MARKER_SIZE + 2 * PROJECT_INDEXES_COUNT);    // one version marker, two series on indexes for project references
constexpr const int SINGLE_PROJECT_SIZE = alignToWord32Boundary(sizeof(SequencerProject));
constexpr const int ALL_PROJECTS_SIZE = PROJECTS_MARKERS_SIZE + (MAX_PROJECTS + 1) * SINGLE_PROJECT_SIZE;
constexpr const int SETTINGS_OFFSET = PROJECTS_OFFSET + alignToWord32Boundary(ALL_PROJECTS_SIZE);

#endif


// look in several spots in the FlashROM to find a valid SettingsBootBlock:
// originally this was always found at the start of the second Flash bank, but as our software
// grows it may be anywhere beyond that point: it may exist at another 'Flash Page'
// start.
//
// Note: this 'can be anywhere' approach also facilitates Flash Wear Leveling as
// we can write any new Configuration/Project chunks anywhere in the Flash:
// our locator will dig up the latest (undamaged) one!

enum AppDataFlashBlockTypeIdentifier {
	NukedFlashBlockTypeId = 0,
	BootFlashBlockTypeId,
	ConfigurationFlashBlockTypeId,
	SequencerProjectFlashBlockTypeId,
};

struct FlashBlockHeader {
	uint16_t typeId;  // AppDataFlashBlockTypeIdentifier
	uint16_t firmwareVersionHash;
	uint16_t size;
	uint16_t headerChecksum;
};

struct FlashBlockFooter {
	uint32_t blockChecksum;
};

uint32_t locateSettingsBootBlock() {
	DEBUGPRINT_FUNCNAME();

	uint32_t pos = IFLASH0_SIZE;
	//byte bootblock = dueFlashStorage.read(pos);
	//settingsBootBlockOffset;  // getFirstFreeBlock



	// TODO


	return 0;
}

AppDataFlashStorage::AppDataFlashStorage()
  : DueFlashStorage(),
    activeBlockStartOffset(0) {}

#if 01

void AppDataFlashStorage::factoryReset() {
}

byte AppDataFlashStorage::readFirstTimeMarker() const {
	return 255;
}

void AppDataFlashStorage::clearFirstTimeMarker() {
}

void AppDataFlashStorage::writePartialData(void *dataBufferOffset, const void* buff, uint32_t actual_size) {
}

const AddressInfo AppDataFlashStorage::getProjectAddressInfo(uint16_t projectId) const {
	return {
		.address = nullptr,
		.size = 0
	};
}

AddressInfo AppDataFlashStorage::allocateProjectStorageSpace(uint16_t projectId, uint32_t size) {
	return {
		.address = nullptr,
		.size = 0
	};
}

const AddressInfo AppDataFlashStorage::getSettingsAddressInfo() const {
	return {
		.address = nullptr,
		.size = 0
	};
}

AddressInfo AppDataFlashStorage::allocateSettingsStorageSpace(uint32_t size) {
	return {
		.address = nullptr,
		.size = 0
	};
}


	const AddressInfo AppDataFlashStorage::getConfigPresetAddressInfo(uint16_t presetId) const {
	return {
		.address = nullptr,
		.size = 0
	};
}

	AddressInfo AppDataFlashStorage::allocateConfigPresetStorageSpace(uint16_t presetId, uint32_t size) {
	return {
		.address = nullptr,
		.size = 0
	};
}

   void AppDataFlashStorage::markSectionAsValid(const AddressInfo &chunkInfo) {
	 }

#else

void writeAdaptivelyToFlash(uint32_t offset, const void* source_, int length) {
  const byte* source = (const byte *)source_;

  // batch and slow down the flash storage in low power mode
  if (Device.operatingLowPower) {
    unsigned long now = millis();

    // ensure that there's at least 50 milliseconds between refreshing the display lights and writing to flash
    unsigned long displayModeDelta = calcTimeDelta(now, displayModeStart);
    if (displayModeDelta < 50) {
      delayUsec((50 - displayModeDelta) * 1000);
    }

    // write the configuration data
    byte batchsize = 128;
    int total = length;
    int i = 0;
    while (i+batchsize < total) {
      appDataFlashStorage.write(offset+i, source+i, batchsize);
      i += batchsize;
      delayUsec(100);
    }

    int remaining = total - i;
    if (remaining > 0) {
      appDataFlashStorage.write(offset+i, source+i, remaining);
    }
    delayUsec(100);
  }
  // do the faster possible flash storage in regular power mode
  else {
    appDataFlashStorage.write(offset, source, length);
  }
}


void AppDataFlashStorage::factoryReset() {
	super::write(0, 254);
}

byte AppDataFlashStorage::readFirstTimeMarker() const {
	byte bootblock = super::read(0);

	if (01 || bootblock != 0) {      // See if we need to boot from scratch
		if (01 || bootblock == 255) {  // When a new firmware is uploaded, the first flash byte will be 255
			                             // Start in serial mode after OS upgrade to be able to receive the settings
		} else {
			// Start in MIDI mode for all other bootblock values
		}
	}

	return bootblock;
}

void AppDataFlashStorage::clearFirstTimeMarker() {
	super::write(0, 0);  // Zero out the firstTime location.
}

void AppDataFlashStorage::writePartialData(void *dataBufferOffset, const void* buff, uint32_t actual_size) {
	super::write(dataBufferOffset, (const byte*)buff, actual_size);
}

const AddressInfo AppDataFlashStorage::getProjectAddressInfo(uint16_t projectId) const {
	uint32_t projectOffset = SETTINGS_OFFSET;

	byte marker = appDataFlashStorage.getActiveProjectsChunkMarker();

	byte prjIndex = super::read(PROJECT_INDEX_OFFSET(marker, projectId));
	uint32_t projectOffset = PROJECTS_OFFSET + PROJECTS_MARKERS_SIZE + prjIndex * SINGLE_PROJECT_SIZE;

	byte* src = (byte*)super::readAddress(projectOffset);

	return {
		.address = src,
		.size = projectSize
	};
}

AddressInfo AppDataFlashStorage::allocateProjectStorageSpace(uint16_t projectId, uint32_t size) {
	byte marker = super::read(PROJECTS_OFFSET);
	byte prjIndex = super::read(PROJECT_INDEX_OFFSET(marker, projectId));
	uint32_t projectOffset = PROJECTS_OFFSET + PROJECTS_MARKERS_SIZE + prjIndex * SINGLE_PROJECT_SIZE;

	byte* src = (byte*)super::readAddress(projectOffset);

	return {
		.address = src,
		.size = projectSize
	};
}

const AddressInfo AppDataFlashStorage::getSettingsAddressInfo() const {
	return {
		.address = nullptr,
		.size = 0
	};
}

AddressInfo AppDataFlashStorage::allocateSettingsStorageSpace(uint32_t size) {
	return {
		.address = nullptr,
		.size = 0
	};
}

   void AppDataFlashStorage::markSectionAsValid(const AddressInfo &chunkInfo) {
	 }

#endif
