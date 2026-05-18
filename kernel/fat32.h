#ifndef _FAT32_H_
#define _FAT32_H_

#include "disk.h"
#include "io.h"
#include <stdint.h>

typedef struct FAT32_Metadata FAT32_Metadata;
struct FAT32_Metadata {
    int sectorPerCluster;
    uint32_t rootCluster;
    int fatLBA;
    int fstClusterLBA;
    int sectorPerFat;
    int nbFats;
    drive_info hardDrive;
};


void setupDrive(void);
typedef struct FAT32_entry FAT32_entry;
struct FAT32_entry{
    unsigned char name[13];
    uint8_t attr;
    uint32_t fstCluster;
    uint32_t size;
};

void readFile(FAT32_Metadata infoFat, uint32_t dirCluster, uint8_t* buffer, int bufferSize);
int isValidEntry(FAT32_entry entry);
int isDirectory(FAT32_entry entry);
int isRemovedEntry(FAT32_entry entry);
void setupDrive();
void readBootSector(drive_info hardDrive, uint32_t partitionAddr, FAT32_Metadata* infoFat);
void getMetadataFileFromDirectory(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index, FAT32_entry *entry);
void mkdir(FAT32_Metadata infoFat, uint32_t dirCluster, char *directoryName);

#endif
