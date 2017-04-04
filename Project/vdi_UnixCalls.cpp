/**
Noah Johnson
Step 0, Unix System calls for vdi files

* make it a class.
* method definitions.

notes:
VDI File Structure
Header
|
map
|
disk

* google all about VDI's 
	-Image signature = magic number
	-offset block (tells you where map is) and offset data

* Step 0 works if we can decypher the disk
	-Master boot record, 55AA (magic number?)
	-byte 4 = filesystem type
	-divide by 2 ^ 20 quotient and remainder are important.

* master boot records
	-EXT 2
	[1st 1k is boot data
         2nd 1k is superblock] (both fixed size in fixed location)

* blockgroup has
	-inodes (128 bytes per inode)
	-1 block for block bitmap
	-no gaps in inodes, one big array
	-data (5 things total)
	-blockroups 0, 1, n = 3, 5, 7, or 0 ^ k have copies of superblock.

* blockgroup discriptor table 
	-only where there is a copy of the superblock, 1k, 2k

read superblock -> figure out block size

fetch block function, unsigned character, 8 bits

each entry in group descritor table, 32 bits, lots of useful information.


vdi_open()

vdi_close()

vdi_read()

vdi_write()

vdi_lseek()

translate()

*/

#include "vdiUnixCalls.h"
#include "boot.h"
#include "datatypes.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

//VDI Header
struct vdiHeader
{
    //int headerStructSize = 164;
    char name[64];
    unsigned int magicNumber;  //Image Signature
    unsigned int version;      
    unsigned int headerSize;   //
    unsigned int imageType;
    unsigned int imageFlags;
    char imageDescription[256]; //
    unsigned int offsetBlocks; //how far into file the map is
    unsigned int offsetData;   //first page frame location
    unsigned int Cylinders;
    unsigned int Heads;
    unsigned int Sectors;      //
    unsigned int sectorSize;   //
    unsigned int unused;
    unsigned long long int diskSize;     //total size of virtual drive
    unsigned int blockSize;    //page size
    unsigned int blockExtraData;
    unsigned int blocksInHDD;
    unsigned int blocksAllocated;
    //unsigned short int majorVersion;
    //unsigned short int minorVersion;
    
    //Ignore UUID's   
};


class vdiFile
{
    int fd;
    int *map;
    int cursor;
    
    
    
    public:
        vdiHeader header;
        MasterBootRecord MBR;
        int vdi_open(const char*);
        void vdi_close();
        ssize_t vdi_read(/*int,*/ void*, size_t);
        ssize_t vdi_write(int, const void*, size_t);
        off_t vdi_lseek(off_t, int);
        void setHeader();
        void translate();
        
};

int main()
{

    unsigned int data[512];

    vdiFile testFile;    

    testFile.vdi_open("/home/csis/Downloads/Good/Test-fixed-1k.vdi");

    testFile.vdi_read(&testFile.MBR, 512);

    cout << "Name: " << testFile.header.name << endl;
    cout << "Magic Number: " << hex << testFile.header.magicNumber << dec << endl;
    cout << "header Size: " << testFile.header.headerSize << endl;
    cout << "Image Description: " << testFile.header.imageDescription << endl;
    cout << "Offset Blocks: " << testFile.header.offsetBlocks << endl;
    cout << "Offset Data: " << testFile.header.offsetData << endl;
    cout << "Sectors: " << testFile.header.Sectors << endl;
    cout << "Sector Size: " << testFile.header.sectorSize << endl;
    cout << "Disk Size: " << testFile.header.diskSize << endl;
    cout << "Block Size: " << testFile.header.blockSize << endl;
    cout << "Blocks in HDD: " << testFile.header.blocksInHDD << endl;
    
    for (int i = 0; i < sizeof(testFile.MBR.unused0); i ++)
    {
        //cout << "Code from Read: " << testFile.MBR.unused0[i] << endl;
    }
    

    for (int i = 0; i < 4; i ++)
    {
        //cout << "Partition Table: " << testFile.MBR.partitionTable[i].type << endl;
    }
    
    cout << "Signature: " << hex << testFile.MBR.magic << dec << endl;
    
    return 0;
}

int vdiFile::vdi_open(const char *pathname)
{
    fd = open(pathname, O_RDONLY);
    //cout << fd << endl;
    if (fd < 0)
    {
        return 1;
    }
    //read header
    //lseek(fd, 1, SEEK_SET);
    read(fd, &header, sizeof(header));  
    map = new int[header.blocksInHDD];
    //lseek(fd, 0, SEEK_SET);
    lseek(fd, header.offsetBlocks, SEEK_SET);
    read(fd, map, 4*header.blocksInHDD);
    cout << "MapZero: " << "0" << " = " << map[0] << endl;
    for (int i=0; i < header.blocksInHDD; i ++)
        {
         //cout << "Map: " << i << " = " << map[i] << endl;
        }
    //cout << "Name2: " << header.name << endl;
    //translate();
    cursor = 0;
    return 0;
}

void vdiFile::vdi_close()
{
    close(fd);
}
        
ssize_t vdiFile::vdi_read(/*const char *fn,*/ /*int fd,*/ void *buf, size_t count)
{
    //fd = file discriptor, read this.
    //*buf = buffer what is read goes here.
    //count = how far to read into file.
    //info = read(fd)
    //fd = open(*fn /*, O_RDONLY*/);
    //lseek the starting point
    translate();
    //lseek(fd, header.offsetData, SEEK_SET);
    read(fd, buf, count);
}        

ssize_t vdiFile::vdi_write(int fd, const void *buf, size_t count)
{
    //write(fd)
    //write(fd, *buf, count);
}

off_t vdiFile::vdi_lseek(off_t offset, int whence)
{
    if (whence == SEEK_SET)
    {
        cursor = offset;
    }
    else if (whence == SEEK_CUR)
    {
        cursor += offset; 
    }
    else if (whence == SEEK_END)
    {
        cursor = header.diskSize + offset;
    } 
    translate();
    //lseek(fd, cursor, whence);
}

void vdiFile::setHeader()
{
    //header.offsetBlocks = 0;
}

void vdiFile::translate()
{
    //int vdiPageSize;
    int pageNumber;
    int offset;
    //int frame;
    int location; 

    cout << "Block Size: " << header.blockSize << endl;
    pageNumber = cursor / header.blockSize;
    cout << "page number: " << pageNumber << endl;
    offset = cursor % header.blockSize;
    cout << "Offset: " << offset << endl;
    location = header.offsetData + (map[pageNumber] * header.blockSize) + offset;
    cout << "location: " << location << endl;
    lseek(fd, location, SEEK_SET);   
}
