#include <stdint.h>
#include "x86-64.h"
#include "fat32.h"
#include "io.h"

drive_info possibleDrives[] = { //primary master is always the boot disk here
{0x1F0, 0xA0},
{0x1F0, 0xB0}, //primary slave
{0x170, 0xA0}, //secondary master
{0x170, 0xB0}  //secondary slave
}; 

// Wait until the ATA status register says ready (0x40 is on)
// & not busy (0x80 is off)
int pingDisk(drive_info drive){ //Uses ATA to find out if hard drive exists
    uint8_t status;
    outb(drive.channel+6, drive.drive); //we select the hard drive        
    outb(drive.channel+7, 0xEC); //we send and IDENTIFY command
    if (inb(drive.channel) == 0){
        return 0;
    }
    
    do{ //wait until the hard drive is not longer busy
        status = inb(drive.channel+7);
    }while(status & 0x80);
    
    if (status & 0x01){ //if there is an error
        return 0;
    }
    return 1;
}


void setupDrive(){
    int activeDrive;
    char msg[5] = "AAAA";
    for (activeDrive = 0; activeDrive < 3; activeDrive++){
         msg[activeDrive]+=pingDisk(possibleDrives[activeDrive]);
    }
    console_print(0, 0, msg);
}
