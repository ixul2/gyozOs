#include <stdint.h>
#include "x86-64.h"
#include "io.h"
#include "fat32.h"

drive_info possibleDrives[] = { //primary master is always the boot disk here
{.channel=0x1F0, .drive=0xB0}, //primary slave
{.channel=0x170, .drive=0xA0}, //secondary master
{.channel=0x170, .drive=0xB0}  //secondary slave
};

uint8_t diskBuffer[512];

void boot_waitdisk(int DRQ) {
  int status ;
  do{
	status = inb(0x1F7);
  }
  while ((status & 0x80) || (!(status & 0x40)) || (DRQ && !(status & 0x08)));
}

int pingDisk(drive_info drive){ //Uses ATA to find out if hard drive exists
    uint8_t status;
    outb(drive.channel+6, drive.drive);
    outb(drive.channel+7, 0xEC); //we send and IDENTIFY command
    if (inb(drive.channel + 7) == 0){
        return 0;
    }
    
    do{ //wait until the hard drive is not longer busy
        status = inb(drive.channel+7);
    }while(status & 0x80);
    
    if (status & 0x01){ //if there is an error
        return 0;
    }
    for (int i = 0; i < 256; i++) { // discard data
        inw(drive.channel); 
    }
    return 1;
}


void readDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber) {
  boot_waitdisk(false);
  outb(drive.channel + 2, 1); // send `count = 1` as an ATA argument
  outb(drive.channel + 3 , sectorNumber); // send `src_sect`, the sector number
  outb(drive.channel + 4, sectorNumber >> 8);
  outb(drive.channel + 5, sectorNumber >> 16);
  outb(drive.channel + 6, (sectorNumber >> 24) | 0xE0 | (drive.drive & (1 << 4)));
  outb(drive.channel + 7, 0x20); // send the command: 0x20 = read sectors

  boot_waitdisk(true);
  insl(drive.channel, (void *)ramAddr, 512 / 4); // read 128 words (or 512 bytes) from the disk
}

void writeDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber) {
  boot_waitdisk(false);
  outb(drive.channel + 2, 1); // send `count = 1` as an ATA argument
  outb(drive.channel + 3 , sectorNumber); // send `src_sect`, the sector number
  outb(drive.channel + 4, sectorNumber >> 8);
  outb(drive.channel + 5, sectorNumber >> 16);
  outb(drive.channel + 6, (sectorNumber >> 24) | 0xE0 | (drive.drive & (1 << 4)));
  outb(drive.channel + 7, 0x30); // send the command: 0x20 = write sectors

  boot_waitdisk(true);
  outsl(drive.channel, (void *)ramAddr, 512 / 4); // read 128 words (or 512 bytes) from the disk
  
  outb(drive.channel + 7, 0xE7); //flush cache
}

int findPartition(drive_info hardDrive){
	readDiskSector(hardDrive, (uintptr_t) diskBuffer, 0); // we read the Master Boot Record
	for (int i = 0; i < 4; i++){
		if ((diskBuffer[446+16*i+4] == 0x0B) || (diskBuffer[446+16*i+4] == 0x0C)){
			return *((uint32_t *) (diskBuffer + 446 + 16*i + 8)); //start sector of the FAT32 partition
		}
	}
	return 0;
}

void setupDrive(){
    int partitionStart, foundDisk;
    drive_info activeDrive;
    FAT32_Metadata infoFat;
    FAT32_entry entry;
    foundDisk = false;
    for (int activeDriveIndex = 0; activeDriveIndex < 3; activeDriveIndex++){
    	activeDrive = possibleDrives[activeDriveIndex];
        if (pingDisk(activeDrive)){
        	foundDisk = true;
        	break; //we stop when we find the active disk
        }
    }
    if (!foundDisk){
    	fail("No disk found");
    }
    partitionStart = findPartition(activeDrive);
    if (!partitionStart){
    	fail("No partition found on the disk");
    }
    readBootSector(activeDrive, partitionStart, &infoFat);
    getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, 1, &entry);
    while(1);
}
