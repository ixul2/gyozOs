typedef struct drive_info drive_info;
struct drive_info{
	int channel;
	uint8_t drive;
};

void setupDrive();
void readDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber);
void writeDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber);
