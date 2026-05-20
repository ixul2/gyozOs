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

void waitdisk(int DRQ) {
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
    waitdisk(false);
    outb(drive.channel + 2, 1); // send `count = 1` as an ATA argument
    outb(drive.channel + 3 , sectorNumber); // send `src_sect`, the sector number
    outb(drive.channel + 4, sectorNumber >> 8);
    outb(drive.channel + 5, sectorNumber >> 16);
    outb(drive.channel + 6, (sectorNumber >> 24) | 0xE0 | (drive.drive & (1 << 4)));
    outb(drive.channel + 7, 0x20); // send the command: 0x20 = read sectors
    waitdisk(true);
    insl(drive.channel, (void *)ramAddr, 512 / 4); // read 128 words (or 512 bytes) from the disk
}

void writeDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber) {
    waitdisk(false);
    outb(drive.channel + 2, 1); // send `count = 1` as an ATA argument
    outb(drive.channel + 3 , sectorNumber); // send `src_sect`, the sector number
    outb(drive.channel + 4, sectorNumber >> 8);
    outb(drive.channel + 5, sectorNumber >> 16);
    outb(drive.channel + 6, (sectorNumber >> 24) | 0xE0 | (drive.drive & (1 << 4)));
    outb(drive.channel + 7, 0x30); // send the command: 0x20 = write sectors

    waitdisk(true);
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

int partitionStart, foundDisk;
drive_info activeDrive;
FAT32_Metadata infoFat;
uint32_t currentCluster;


void setupDrive(){
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
  currentCluster = infoFat.rootCluster;
  /*getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, 0, &entry); //it's a valid directory
  console_print_int(0, 0, isValidEntry(entry));
  console_print_int(1, 0, isDirectory(entry));*/
  
  
  /*char content[100];
  getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, 0, &entry);
  getMetadataFileFromDirectory(infoFat, entry.fstCluster, 2, &entry);
  readFile(infoFat, entry.fstCluster, content, entry.size);
  console_print(1, 0, content);*/
  
  /*char content[20000];
  getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, 1, &entry);
  getMetadataFileFromDirectory(infoFat, entry.fstCluster, 50, &entry);
  console_print_int(0, 0, entry.size);
  readFile(infoFat, entry.fstCluster, content, entry.size);
  console_print(1, 0, content);*/
  

  /*int i = 0;
  FAT32_entry dir1;
  //removeEntryFromDirectory(infoFat, infoFat.rootCluster, 2);
  while(1){
    getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, i, &entry); //it's a valid directory
    if (!isValidEntry(entry)){
      break;
    }
    if (!isRemovedEntry(entry)){
      console_print(i, 0, entry.name);
    }
    i++;
  }*/
  
  /*FAT32_entry dir1;
  int i = 0;
  getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, 0, &dir1);
  char name[] = "TEST000.   ";
  for (int i = 0; i < 100; i++){
    name[4] = '0'+(i/100)%10;
    name[5] = '0'+(i/10)%10;
    name[6] = '0'+i%10;
    mkdir(infoFat, dir1.fstCluster, name);
    console_print_int(0, i*5, i);
  }
  while(1){
    getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, i, &entry); //it's a valid directory
    if (!isValidEntry(entry)){
      break;
    }
    if (!isRemovedEntry(entry)){
      console_print(0, i*10, entry.name);
      console_print_int(1, i*10, entry.fstCluster);
    }
    i++;
  }
  */
  /*
  FAT32_entry dir1;
  int i = 0;
  char test[1000], read[1001];
  for (int i = 0; i < 1000; i++){
    test[i] = 'A'+i%15;
  }
  writeFile(infoFat, infoFat.rootCluster, "fichier.txt", test, 1000);
  while(1){
    getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, i, &entry); //it's a valid directory
    if (!isValidEntry(entry)){
      break;
    }
    if (!isRemovedEntry(entry)){
      console_print(i, 0, entry.name);
    }
    i++;
  }
  getMetadataFileFromDirectory(infoFat, infoFat.rootCluster, 3, &entry);
  readFile(infoFat, entry.fstCluster, read, 1000);
  read[1000] = '\0';
  console_print(10, 0, read);*/
}


int file_ind = 0;

int list_files(char* buffer){
    FAT32_entry entry;
    while(1){
        getMetadataFileFromDirectory(infoFat, currentCluster, file_ind, &entry);
        if (!isValidEntry(entry)){
            file_ind = 0;
            return -1;
        }
        if(!isRemovedEntry(entry)){
            strcpy(buffer,entry.name);
            file_ind++;
            return (file_ind-1);
        }
        file_ind++;
    }
}
void make_directory(char* name){
    mkdir(infoFat, currentCluster, name);
}

void remove_directory(int ind){
    removeEntryFromDirectory(infoFat, currentCluster, ind);
}

void change_directory(int ind){
    FAT32_entry entry;
    getMetadataFileFromDirectory(infoFat, currentCluster, ind, &entry);
    currentCluster = entry.fstCluster;
    file_ind = 0;
}

void read_file(int ind, char* buffer){
    FAT32_entry entry;
    getMetadataFileFromDirectory(infoFat, currentCluster, ind, &entry);
    readFile(infoFat, entry.fstCluster, buffer, entry.size);
}

void write_file(char* content, int len, char* name){
    writeFile(infoFat, currentCluster, name, content, len);
}