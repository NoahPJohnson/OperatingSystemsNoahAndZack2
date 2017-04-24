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
#include "ext2.h"
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
        ext2_super_block superBlock;
        ext2_group_desc* blockGroupDescriptorTable;
        int vdi_open(const char*);
        void vdi_close();
        ssize_t vdi_read(/*int,*/ void*, size_t);
        ssize_t vdi_write(int, const void*, size_t);
        off_t vdi_lseek(off_t, int);
        void setHeader();
        void translate();
        
};

class blockClass
{
    int blockSize;
    int freeSpace;
    int usedSpace;    
    int partitionStart;
    int blockGroups;
    int maxBlocks;
    int groupDescriptorPerBlock;
    int groupDTBlocksUsed;
    int groupDTBlocks;
    int inodesPerBlock;
    int addressesPerBlock;
    int numberOfFiles;
    int numberOfDirectories;

    public:
        void fetchSuperblock(vdiFile*);
        void fetchBlock(int, vdiFile*, ext2_inode*);
        void fetchInode(int, vdiFile*);
        void fetchBlockGroupDescriptorTable(vdiFile*);
        bool checkBGDT(vdiFile*);
        void calculateMaxBlocks(ext2_super_block*);
        void calculateGDTBlocks(vdiFile*);
        void calculateSpace(ext2_super_block*);   
        void calculateFiles(vdiFile*);
        int getFreeSpace();
        int getUsedSpace();
        void displayFileInformation(vdiFile*);
};

int main()
{

    

    vdiFile testFile;    
    blockClass blockLayer;    

    testFile.vdi_open("/home/csis/Downloads/Good/Test-fixed-1k.vdi");

    char data[testFile.header.blockSize];    

    testFile.vdi_read(&testFile.MBR, 512);

    blockLayer.fetchSuperblock(&testFile);

    //testFile.vdi_lseek((testFile.MBR.partitionTable[0].firstSector*512)+1024, SEEK_SET);

    //testFile.vdi_read(&testFile.superBlock, 1024);

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
    
    cout << "Inodes: " << testFile.superBlock.s_inodes_count << endl;
    cout << "SuperBlock: " << hex << testFile.superBlock.s_magic << dec << endl;
    cout << "First Inode: " << testFile.superBlock.s_first_ino << endl;

    for (int i = 0; i < sizeof(testFile.MBR.unused0); i ++)
    {
        //cout << "Code from Read: " << testFile.MBR.unused0[i] << endl;
    }
    

    for (int i = 0; i < 4; i ++)
    {  
        cout << "Partition Table Type: " << testFile.MBR.partitionTable[i].type << endl; 
        cout << "Partition Table: " << testFile.MBR.partitionTable[i].firstSector << endl;
    }
    
    blockLayer.displayFileInformation(&testFile);

    //cout << endl;
    //cout << "First inode: " << testFile.superBlock.s_first_ino << endl;
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
    //lseek(fd, 2098176, SEEK_SET);
    //read(fd, &superBlock, sizeof(superBlock));
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
    //translate();
    //cursor = offset + header.offsetData;
    //lseek(fd, cursor, SEEK_SET);
    //read(fd, &superBlock, sizeof(superBlock));
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

    cout << "Cursor: " << cursor << endl; 
    cout << "Block Size: " << header.blockSize << endl;
    pageNumber = cursor / header.blockSize;
    cout << "page number: " << pageNumber << endl;
    offset = cursor % header.blockSize;
    cout << "Offset: " << offset << endl;
    location = header.offsetData + (map[pageNumber] * header.blockSize) + offset;
    cout << "location: " << location << endl;
    lseek(fd, location, SEEK_SET);   
}

void blockClass::fetchSuperblock(vdiFile* file)
{
    int partitionTableIndex;
    for (int i = 0; i < 4; i++)
    {
        if (file->MBR.partitionTable[i].type == 0x83)
        {
            partitionTableIndex = i;     
        }
    }
    partitionStart = (file->MBR.partitionTable[partitionTableIndex].firstSector*512);
    file->vdi_lseek(partitionStart+1024, SEEK_SET);
    //cout << (file->MBR.partitionTable[0].firstSector*512)+1024 << endl;
    file->vdi_read(&file->superBlock, 1024);
    //file->vdi_read(&file->
    blockSize = 1024 << file->superBlock.s_log_block_size;
    blockGroups = (file->superBlock.s_blocks_count + file->superBlock.s_blocks_per_group - 1) / file->superBlock.s_blocks_per_group;
    fetchBlockGroupDescriptorTable(file);
    calculateMaxBlocks(&file->superBlock);
    addressesPerBlock = blockSize / sizeof(u32);
    calculateGDTBlocks(file);
    inodesPerBlock = blockSize / file->superBlock.s_inode_size;
    
    calculateSpace(&file->superBlock);
    calculateFiles(file);
    fetchInode(2, file);
    //cout << hex << file->superBlock.s_magic << dec << endl;
}

void blockClass::fetchBlockGroupDescriptorTable(vdiFile* file)
{
    file->vdi_lseek((1+file->superBlock.s_first_data_block)*blockSize+partitionStart, SEEK_SET);
    
    file->blockGroupDescriptorTable = new ext2_group_desc[blockGroups];
    
    for (int i = 0; i < blockGroups; i++)
    {
         file->vdi_read(&file->blockGroupDescriptorTable[i], sizeof(file->blockGroupDescriptorTable[i]));
         file->vdi_lseek(sizeof(file->blockGroupDescriptorTable[i]), SEEK_CUR);
    }
}

bool blockClass::checkBGDT(vdiFile* file)
{
    int BGDTFreeBlockTotal = 0;
    int BGDTFreeInodeTotal = 0;
    for (int i = 0; i < blockGroups; i++)
    {
         BGDTFreeBlockTotal += file->blockGroupDescriptorTable[i].bg_free_blocks_count;
         BGDTFreeInodeTotal += file->blockGroupDescriptorTable[i].bg_free_inodes_count;
    }
    if (file->superBlock.s_free_blocks_count != BGDTFreeBlockTotal)
    {
        return false;
    }
    if (file->superBlock.s_free_inodes_count != BGDTFreeInodeTotal)
    {
        return false;
    }
    return true;
}

void blockClass::fetchBlock(int blockIndex, vdiFile* file, ext2_inode* buf)
{
    file->vdi_lseek((partitionStart)+blockIndex*blockSize, SEEK_SET);
    cout << endl;
    cout << "SIZE OF BUF  " << sizeof(buf[0]) << endl; 
    cout << endl;
    for (int i = 0; i < inodesPerBlock; i ++)
    {
         file->vdi_read(&buf[i], sizeof(buf[i]));
         file->vdi_lseek(sizeof(buf[i]), SEEK_CUR);
    }
}

void blockClass::fetchInode(int inodeIndex, vdiFile* file)
{
    inodeIndex--;
    int groupNumber;
    groupNumber = (inodeIndex / file->superBlock.s_inodes_per_group);
    inodeIndex = (inodeIndex % file->superBlock.s_inodes_per_group);
    __le32 startingBlock = file->blockGroupDescriptorTable[groupNumber].bg_inode_table;
    int blockNumber = startingBlock+inodeIndex/inodesPerBlock;
    inodeIndex = inodeIndex % inodesPerBlock;
    //Fetch block b         b=blockNumber
    ext2_inode* block = new ext2_inode[inodesPerBlock];
    cout << endl;
    cout << "size of block: " << sizeof(block[0]) << endl;
    cout << "inode index: " << inodeIndex << endl;
    fetchBlock(blockNumber, file, block);
    //b[i] is the inode.
    ext2_inode inode = block[inodeIndex];
    cout << "Inode Size: " << inode.i_size << endl;
    cout << "Inode Blocks: " << inode.i_blocks << endl;
    cout << "Inode Links: " << inode.i_links_count << endl;
}

void blockClass::calculateMaxBlocks(ext2_super_block* sBlock)
{
    maxBlocks = 0x003fffff;
    if (maxBlocks > sBlock->s_blocks_count)
    {
	maxBlocks = sBlock->s_blocks_count;
    }
    maxBlocks <<= 10;
    maxBlocks = (maxBlocks + sBlock->s_blocks_per_group - 1 - sBlock->s_first_data_block) / sBlock->s_blocks_per_group;
}

void blockClass::calculateGDTBlocks(vdiFile* file)
{
    groupDescriptorPerBlock = blockSize / (sizeof(struct ext2_group_desc));
    //cout << "GDPB: " << groupDescriptorPerBlock << endl;
    groupDTBlocksUsed = (blockGroups + groupDescriptorPerBlock - 1) / groupDescriptorPerBlock;
    //cout << "GDPBUsed: " << groupDTBlocksUsed << endl;
    groupDTBlocks = ((maxBlocks + groupDescriptorPerBlock - 1) / groupDescriptorPerBlock) - groupDTBlocksUsed;
    //cout << "GDPBlocks before if: " << groupDTBlocks << endl;
    if (groupDTBlocks > addressesPerBlock)
    {
	groupDTBlocks = addressesPerBlock;
    }
    //cout << "GDPBlocks after if: " << groupDTBlocks << endl;
    groupDTBlocks += groupDTBlocksUsed;
    //cout << "GDPBlocks: " << groupDTBlocks << endl;
}

void blockClass::calculateSpace(ext2_super_block* sBlock)
{
    int space = (sBlock->s_blocks_count * blockSize);
    freeSpace = ((sBlock->s_free_blocks_count + sBlock->s_r_blocks_count) * blockSize);
    usedSpace = (space - freeSpace);
}

void blockClass::calculateFiles(vdiFile* file)
{
    numberOfFiles = 0;
    numberOfDirectories = 0;
    for (int i = 0; i < blockGroups; i++)
    {
         numberOfDirectories += file->blockGroupDescriptorTable[i].bg_used_dirs_count;
    }
    numberOfFiles = file->superBlock.s_inodes_count - (numberOfDirectories+file->superBlock.s_free_inodes_count);
}

int blockClass::getFreeSpace()
{
    return freeSpace;
}

int blockClass::getUsedSpace()
{
    return usedSpace;
}

void blockClass::displayFileInformation(vdiFile* file)
{
    cout << "MBR Signature: " << hex << file->MBR.magic << dec << endl;
    
    cout << endl;
    cout << "File System Size: " << file->header.diskSize << endl;
    
    cout << endl;
    cout << "Space Available:  " << getFreeSpace() << endl;
    
    cout << endl;
    cout << "Used Space:       " << getUsedSpace() << endl;
    

    cout << endl;
    cout << "Number of Blocks: " << " Total - " << file->superBlock.s_blocks_count << " Free - " << file->superBlock.s_free_blocks_count << 
                                    " Reserved - " << file->superBlock.s_r_blocks_count << endl;
    cout << "Number of Inodes: " << " Total - " << file->superBlock.s_inodes_count << " Free - " << file->superBlock.s_free_inodes_count << endl;
    
    cout << endl;
    cout << "Match w/ Bitmap:  " << checkBGDT(file) << endl;
    
    cout << endl;
    cout << "First Data Block: " << file->superBlock.s_first_data_block << endl;
    
    cout << endl;
    cout << "Block Size:       " << blockSize << endl;

    cout << endl;
    cout << "Block Groups:     " << blockGroups << endl;

    cout << endl;
    cout << "Blocks Per Group: " << file->superBlock.s_blocks_per_group << endl;
	
    cout << endl;
    cout << "Inodes Per Group: " << file->superBlock.s_inodes_per_group << endl;

    cout << endl;
    cout << "GDT Blocks      : " << groupDTBlocks << endl;

    cout << endl;
    cout << "Inodes Per Block: " << inodesPerBlock << endl;

    cout << endl;
    cout << "Addresses Per Bk: " << addressesPerBlock << endl;

    cout << endl;
    for (int i = 0; i < blockGroups; i++)
    {
        cout << "Group: " << i << endl;
        cout << endl;
	cout << "Block Bitmap: " << file->blockGroupDescriptorTable[i].bg_block_bitmap << endl;
	cout << "Inode Bitmap: " << file->blockGroupDescriptorTable[i].bg_inode_bitmap << endl;
	cout << "Inode Table:  " << file->blockGroupDescriptorTable[i].bg_inode_table  << endl;
	cout << "Free Blocks:  " << file->blockGroupDescriptorTable[i].bg_free_blocks_count << endl;
	cout << "Free Inodes:  " << file->blockGroupDescriptorTable[i].bg_free_inodes_count << endl;
        cout << endl;
        cout << endl;
    }

    cout << endl;
    cout << "Max Blocks:       " << maxBlocks << endl;

    cout << endl;
    cout << "Files Count:      " << numberOfFiles << endl;

    cout << endl;
    cout << "Directories Cnt:  " << numberOfDirectories << endl;
 
    cout << endl;
    cout << "FileSystem State: " << file->superBlock.s_state << endl;
}



