#include "disk.h"

typedef struct FAT32_Metadata FAT32_Metadata;
struct FAT32_Metadata {
	int sectorPerCluster;
	int rootCluster;
	int fatLBA;
	int fstClusterLBA;
	int sectorPerFat;
	int nbFats;
	drive_info hardDrive;
};

typedef struct FAT32_entry FAT32_entry;
struct FAT32_entry{
	char name[13];
	uint8_t attr;
	uint32_t fstCluster;
	uint32_t fileSize;
};

void readFile(FAT32_Metadata infoFat, uint32_t dirCluster, uint8_t* buffer, int bufferSize);

void setupDrive();
void readDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber);
void writeDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber);
