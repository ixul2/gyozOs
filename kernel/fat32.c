#include <stdint.h>
#include "fat32.h"
#include "io.h"

extern uint8_t diskBuffer[512];


void readBootSector(drive_info hardDrive, uint32_t partitionAddr, FAT32_Metadata* infoFat){
	int sectorsPerCluster, rootCluster, numberReservedSectors, sectorPerFat, nbFats;
	readDiskSector(hardDrive, (uintptr_t) diskBuffer, partitionAddr); // we read the boot sector
	if ((*((uint16_t *) (diskBuffer + 0x1FE))) != 0xAA55){ //we check the signature
		fail("Invalid partition");
	}
	if ((*((uint16_t *) (diskBuffer + 0x0B))) != 512){
		fail("This OS only supports hard drives with sectors of 512 bytes");
	}
	sectorsPerCluster = (*((uint8_t *) (diskBuffer + 0x0D)));
	rootCluster = (*((uint32_t *) (diskBuffer + 0x2C)));
	numberReservedSectors = (*((uint16_t *) (diskBuffer + 0x0E)));
	sectorPerFat = (*((uint32_t *) (diskBuffer + 0x24)));
	nbFats = (*((uint8_t *) (diskBuffer + 0x10)));
	
	infoFat -> sectorPerCluster = sectorsPerCluster;
	infoFat -> rootCluster = rootCluster;
	infoFat -> fatLBA = partitionAddr + numberReservedSectors; 
	infoFat -> sectorPerFat = sectorPerFat;
	infoFat -> nbFats = nbFats;
	infoFat -> fstClusterLBA = infoFat -> rootCluster + sectorPerFat * nbFats;
	infoFat -> hardDrive = hardDrive;
}

void updateEveryFat(FAT32_Metadata infoFat, int sectorFirstFat){
	for (int i = 0; i < infoFat.nbFats; i++){ //we update every FAT table
		writeDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, sectorFirstFat + i * infoFat.sectorPerFat);
	}
}

int allocateFreeCluster(FAT32_Metadata infoFat, int lastCluster){
	int indexSector = infoFat.fatLBA;
	int clusterIndex = 2;
	while(indexSector < infoFat.sectorPerFat){
	    readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, indexSector);
	    while(clusterIndex%16 != 0){
	        if (((uint32_t *)diskBuffer) [clusterIndex%16] == 0){
	            ((uint32_t *)diskBuffer)[clusterIndex%16] = 0xFFFFFFFF;
	            updateEveryFat(infoFat, indexSector);
	            readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, infoFat.fatLBA + (lastCluster >> 7));
	            *(((uint32_t *) diskBuffer) + (lastCluster%128)) = clusterIndex;
	            updateEveryFat(infoFat, infoFat.fatLBA + (lastCluster >> 7));
	            return clusterIndex;
	        }
	        clusterIndex++;
	    }
	    indexSector++;
	}
	fail("no new cluster can be allocated");
}

int nextCluster(FAT32_Metadata infoFat, uint32_t dirCluster){
	int newDirCluster;
	readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, infoFat.fatLBA + (dirCluster >> 7));
	newDirCluster = *(((uint32_t *) diskBuffer) + (dirCluster%128));
	if (newDirCluster == 0xFFFFFFFF){ //if it's the last cluster we allocate a new one
        return allocateFreeCluster(infoFat, dirCluster);
	}
	return newDirCluster;
}

void readFileName(char *fat32Name, char* filename){
	int index = 7;
	while((fat32Name[index] = ' ') && (index >= 0)){ //we strip the trailing spaces
		index--;
	}
	for (int i = 0; i <= index; i++){
		fat32Name[i] = filename[i];
	}
	filename[index+1] = '.';
	for (int i = 0; i < 3; i++){
		filename[index+i+2] = fat32Name[8+i];
	}
	filename[index+5] = '\0';
}

void writeFileName(char *fat32Name, char* filename){
	int index = 7;
	while((fat32Name[index] = ' ') && (index >= 0)){ //we strip the trailing spaces
		index--;
	}
	for (int i = 0; i <= index; i++){
		fat32Name[i] = filename[i];
	}
	filename[index+1] = '.';
	for (int i = 0; i < 3; i++){
		filename[index+i+2] = fat32Name[8+i];
	}
	filename[index+5] = '\0';
}

int isValidEntry(FAT32_entry entry){
	if((entry.name[0] == 0) || (entry.name[0] == 0xE5)){ //if it's marked as end of directory or unused
		return 0;
	}
	return 1;
}

int isDirectory(FAT32_entry entry){
	return entry.attr & (1 << 4);
}

void getToRightRecord(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index, uint8_t **record, int *sector){
	int indexSector, indexCluster;
	indexSector = (index/32); //there are 32 recods in each sector
	indexSector %= (infoFat.sectorPerCluster);
	indexCluster = indexSector/(infoFat.sectorPerCluster);
	for (int i = 0; i < indexCluster; i++){ //we use the FAT table to find the right cluster
		nextCluster(infoFat, dirCluster);
	}
	*sector = infoFat.fstClusterLBA + (infoFat.sectorPerCluster * (dirCluster-2)) + indexSector;
	readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, *sector); //we copy the right sector
	*record = diskBuffer + (index%32)*16; //this is where the record is
}

void getMetadataFileFromDirectory(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index, FAT32_entry *entry){ 
	uint8_t *record;
	int sector;
	getToRightRecord(infoFat, dirCluster, index, &record, &sector);
	readFileName(record, entry -> name);
	entry -> attr = record[11];
	entry -> fstCluster = *((uint16_t*)(record + 20)) << 16;
	entry -> fstCluster += *((uint16_t*)(record + 26));
	entry -> fileSize += *((uint32_t*)(record + 28));
}

void addFileToDirectory(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index, FAT32_entry *entry){ 
	uint8_t *record;
	int sector;
	getToRightRecord(infoFat, dirCluster, index, &record, &sector);
	writeFileName(record, entry -> name);
	record[11] = entry -> attr;
	*((uint16_t*)(record + 20)) = (entry -> fstCluster) >> 16;
	*((uint16_t*)(record + 26)) = entry -> fstCluster;
	*((uint32_t*)(record + 28)) = (entry -> fileSize) % (1<<16);
	updateEveryFat(infoFat, sector);
}

void removeEntryFromDirectory(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index){
	uint8_t *record;
	int sector;
	getToRightRecord(infoFat, dirCluster, index, &record, &sector);
	record[0] = 0xE5;
	updateEveryFat(infoFat, sector);
}


void readFile(FAT32_Metadata infoFat, uint32_t dirCluster, uint8_t* buffer, int bufferSize){
	int indexSector = 0, bufferIndex = 0;
	while(bufferSize){
		int copySize = (bufferSize>=512) ? 512 : bufferSize;
		readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, infoFat.fstClusterLBA + (infoFat.sectorPerCluster * (dirCluster-2)) + indexSector); //we copy the right sector
		for (int i = 0; i < copySize; i++){
			buffer[bufferIndex] = diskBuffer[i];
		}
		bufferSize -= copySize;
		indexSector++;
		if (indexSector >=infoFat.sectorPerCluster){
			dirCluster = nextCluster(infoFat, dirCluster);
		}
	}
}
