#include <stdint.h>
#include "fat32.h"
#include "io.h"

extern uint8_t diskBuffer[512];

int isValidEntry(FAT32_entry entry){
	if(entry.name[0] == 0){ //if it's marked as end of directory
		return 0;
	}
	return 1;
}

int isRemovedEntry(FAT32_entry entry){
	if(entry.name[0] == 0xE5){ //if it's marked as unused
		return 1;
	}
	return 0;
}

int isDirectory(FAT32_entry entry){
	return entry.attr & (1 << 4);
}

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
	infoFat -> fstClusterLBA = infoFat -> fatLBA + sectorPerFat * nbFats;
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
	while(indexSector < infoFat.fatLBA + infoFat.sectorPerFat){
	    readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, indexSector);
	    while(clusterIndex%16 != 0){
	        if (((uint32_t *)diskBuffer)[clusterIndex%16] == 0){
	            ((uint32_t *)diskBuffer)[clusterIndex%16] = 0x0FFFFFF8;
	            updateEveryFat(infoFat, indexSector);
	            for (int i = 0; i < 512; i++){ 
	            	diskBuffer[i] = 0;
	            }
	            for (int i = 0; i < infoFat.sectorPerCluster; i++){ //we wipe the whole cluster
	            	int sectorNumber = infoFat.fstClusterLBA + (infoFat.sectorPerCluster * (clusterIndex-2)) + i;
	            	writeDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, sectorNumber);
	            }
	            
	            if (lastCluster){
			        readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, infoFat.fatLBA + (lastCluster >> 7));
			        *(((uint32_t *) diskBuffer) + (lastCluster%128)) = clusterIndex;
			        updateEveryFat(infoFat, infoFat.fatLBA + (lastCluster >> 7));
	            }
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
	if (newDirCluster >= 0x0FFFFFF8){ //if it's the last cluster we allocate a new one
        return allocateFreeCluster(infoFat, dirCluster);
	}
	return newDirCluster;
}

void readFileName(unsigned char *fat32Name, unsigned char* filename){
	if (fat32Name[0] == '\0'){ //if file name starts with \0 it's a deleted file
		filename[0] = '\0';
	}
	else{
		int index = 7;
		while((fat32Name[index] == ' ') && (index >= 0)){ //we strip the trailing spaces
			index--;
		}
		for (int i = 0; i <= index; i++){
			filename[i] = fat32Name[i];
		}
		if (fat32Name[8] == ' '){ //if there is no extension
			filename[index+1] = '\0';
		}
		else{
			filename[index+1] = '.';
			for (int i = 0; i < 3; i++){
				filename[index+i+2] = fat32Name[8+i];
			}
			filename[index+5] = '\0';
		}
	}
}

void getToRightRecord(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index, uint8_t **record, int *sector){
	int indexSector, indexCluster;
	indexSector = (index/16); //there are 32 recods in each sector
	indexCluster = indexSector/(infoFat.sectorPerCluster);
	indexSector %= (infoFat.sectorPerCluster);
	for (int i = 0; i < indexCluster; i++){ //we use the FAT table to find the right cluster
		dirCluster = nextCluster(infoFat, dirCluster);
	}
	*sector = infoFat.fstClusterLBA + (infoFat.sectorPerCluster * (dirCluster-2)) + indexSector;
	readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, *sector); //we copy the right sector
	*record = diskBuffer + (index%16)*32; //this is where the record is
}

void getMetadataFileFromDirectory(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index, FAT32_entry *entry){ 
	uint8_t *record;
	int sector;
	getToRightRecord(infoFat, dirCluster, index, &record, &sector);
	readFileName(record, entry -> name);
	entry -> attr = record[11];
	entry -> fstCluster = *((uint16_t*)(record + 20)) << 16;
	entry -> fstCluster += *((uint16_t*)(record + 26));
	entry -> size = *((uint32_t*)(record + 28));
}

void readFile(FAT32_Metadata infoFat, uint32_t fileCluster, uint8_t* buffer, int bufferSize){
	int indexSector = 0, bufferIndex = 0;
	while(bufferSize){
		int copySize = (bufferSize>=512) ? 512 : bufferSize;
		readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, infoFat.fstClusterLBA + (infoFat.sectorPerCluster * (fileCluster-2)) + indexSector); //we copy the right sector
		for (int i = 0; i < copySize; i++){
			buffer[bufferIndex] = diskBuffer[i];
			bufferIndex++;
		}
		bufferSize -= copySize;
		indexSector++;
		if (indexSector >=infoFat.sectorPerCluster){
			indexSector = 0;
			fileCluster = nextCluster(infoFat, fileCluster);
		}
	}
}

void writeFileName(char *fat32Name, char* filename){
	int index = 0;
	while ((index < 8) && (filename[index] != '.')){
		fat32Name[index] = filename[index];
	}
	for (int i = index; i < 8; i++){
		fat32Name[i] = ' ';
	}
	for (int i = 0; i < 3; i++){
		fat32Name[8+i] = filename[index+i+1];
	}
}


void addFileToDirectory(FAT32_Metadata infoFat, uint32_t dirCluster, FAT32_entry entry){ 
	int sector, indexSector = 0, bufferIndex = 0, indexRecord = 0;
	uint8_t *record;
	while(1){
		sector = infoFat.fstClusterLBA + (infoFat.sectorPerCluster * (dirCluster-2)) + indexSector;
		readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, sector);
		if (*((uint8_t *) diskBuffer + 32*indexRecord) == 0){ //if we have a free space
			break;
		}
		indexRecord++;
		if (indexRecord >= 16){
			indexRecord = 0;
			indexSector++;
			if (indexSector >=infoFat.sectorPerCluster){
				indexSector = 0;
				dirCluster = nextCluster(infoFat, dirCluster);
			}
		}
	}
	record = (uint8_t *) diskBuffer + 32*indexRecord;
	writeFileName(record, entry.name);
	record[11] = entry.attr;
	*((uint16_t*)(record + 20)) = (entry.fstCluster) >> 16;
	*((uint16_t*)(record + 26)) = entry.fstCluster;
	*((uint32_t*)(record + 28)) = (entry.size) % (1<<16);
	updateEveryFat(infoFat, sector);
}

void mkdir(FAT32_Metadata infoFat, uint32_t dirCluster, char *directoryName){
	int fstCluster;
	FAT32_entry entry;
	fstCluster = allocateFreeCluster(infoFat, 0);
	writeFileName(entry.name, directoryName);
	entry.attr = 16; //directory
	entry.fstCluster = fstCluster;
	entry.size = 0;
	addFileToDirectory(infoFat, dirCluster, entry);
}

void writeFile(FAT32_Metadata infoFat, uint32_t dirCluster, char* filename, uint8_t* buffer, int bufferSize){
	FAT32_entry entry;
	int indexSector = 0, fileCluster, bufferIndex = 0;
	entry.fstCluster = fileCluster = allocateFreeCluster(infoFat, 0);
	while(bufferSize){ //we create the file
		int copySize, sector;
		copySize = (bufferSize>=512) ? 512 : bufferSize;
		sector = infoFat.fstClusterLBA + (infoFat.sectorPerCluster * (fileCluster-2)) + indexSector;
		readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, sector); //we copy the right sector
		for (int i = 0; i < copySize; i++){
			buffer[bufferIndex] = diskBuffer[i];
			bufferIndex++;
		}
		writeDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, sector); //we put it in the hard drive
		bufferSize -= copySize;
		indexSector++;
		if (indexSector >=infoFat.sectorPerCluster){
			fileCluster = nextCluster(infoFat, fileCluster);
		}
	}
	writeFileName(entry.name, filename);
	entry.attr = 0;
	entry.size = bufferSize;
	addFileToDirectory(infoFat, dirCluster, entry);
}

void removeFATChain(FAT32_Metadata infoFat, uint32_t cluster){
	int newCluster, sector;
	sector = infoFat.fatLBA + (cluster >> 7);
	readDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, sector);
	newCluster = *(((uint32_t *) diskBuffer) + (cluster%128)); //we remove the values from the FAT table
	*(((uint32_t *) diskBuffer) + (cluster%128)) = 0;
	updateEveryFat(infoFat, sector);
	if (newCluster < 0x0FFFFFF8){ //if it's not the last cluster
     	removeFATChain(infoFat, newCluster);
	}
}

void removeEntryFromDirectory(FAT32_Metadata infoFat, uint32_t dirCluster, uint32_t index){
	uint8_t *record;
	int sector, fstCluster;
	getToRightRecord(infoFat, dirCluster, index, &record, &sector);
	fstCluster = *((uint16_t*)(record + 20)) << 16;
	fstCluster += *((uint16_t*)(record + 26));
	record[0] = 0xE5;
	writeDiskSector(infoFat.hardDrive, (uintptr_t) diskBuffer, sector);
	removeFATChain(infoFat, fstCluster);
}


