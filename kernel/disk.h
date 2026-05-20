#ifndef _DISK_H_
#define _DISK_H_

#include <stdint.h>
#include "x86-64.h"
#include "io.h"
#include "fat32.h"

typedef struct drive_info{
    int channel;
    uint8_t drive;
} drive_info;

void setupDrive();
void readDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber);
void writeDiskSector(drive_info drive, uintptr_t ramAddr, uint32_t sectorNumber);
int list_files(char*);
void make_directory(char*);
void change_directory(int);
void remove_directory(int);
void write_file(char* content, int len, char* name);
void read_file(int ind, char* buffer);
#endif
