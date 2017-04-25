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
#include "EXT2Functions.h"
#include "boot.h"
//#include "ext2.h"
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
    //int majorVersion;
    //int minorVersion;

    //Ignore UUID's
};

struct ext2_super_block {
    int s_inodes_count; /* Inodes count */
    int s_blocks_count; /* Blocks count */
    int s_r_blocks_count; /* Reserved blocks count */
    int s_free_blocks_count; /* Free blocks count */
    int s_free_inodes_count; /* Free inodes count */
    int s_first_data_block; /* First Data Block */
    int s_log_block_size; /* Block size */
    int s_log_frag_size; /* Fragment size */
    int s_blocks_per_group; /* # Blocks per group */
    int s_frags_per_group; /* # Fragments per group */
    int s_inodes_per_group; /* # Inodes per group */
    int s_mtime; /* Mount time */
    int s_wtime; /* Write time */
    short int s_mnt_count; /* Mount count */
    short int s_max_mnt_count; /* Maximal mount count */
    short int s_magic; /* Magic signature */
    short int s_state; /* File system state */
    short int s_errors; /* Behaviour when detecting errors */
    short int s_minor_rev_level; /* minor revision level */
    int s_lastcheck; /* time of last check */
    int s_checkint32_terval; /* max. time between checks */
    int s_creator_os; /* OS */
    int s_rev_level; /* Revision level */
    short int s_def_resuid; /* Default uid for reserved blocks */
    short int s_def_resgid; /* Default gid for reserved blocks */
    /*
     * These fields are for EXT2_DYNAMIC_REV superblocks only.
     *
     * Note: the difference between the compatible feature set and
     * the incompatible feature set is that if there is a bit set
     * in the incompatible feature set that the kernel doesn't
     * know about, it should refuse to mount the filesystem.
     *
     * e2fsck's requirements are more strict; if it doesn't know
     * about a feature in either the compatible or incompatible
     * feature set, it must abort and not try to meddle with
     * things it doesn't understand...
     */
    int s_first_ino; /* First non-reserved inode */
    short int s_inode_size; /* size of inode structure */
    short int s_block_group_nr; /* block group # of this superblock */
    int s_feature_compat; /* compatible feature set */
    int s_feature_incompat; /* incompatible feature set */
    int s_feature_ro_compat; /* readonly-compatible feature set */
    unsigned char s_uuid[16]; /* 128-bit uuid for volume */
    char s_volume_name[16]; /* volume name */
    char s_last_mounted[64]; /* directory where last mounted */
    int s_algorithm_usage_bitmap; /* For compression */
    /*
     * Performance hints.  Directory preallocation should only
     * happen if the EXT2_COMPAT_PREALLOC flag is on.
     */
    unsigned char s_prealloc_blocks; /* Nr of blocks to try to preallocate*/
    unsigned char s_prealloc_dir_blocks; /* Nr to preallocate for dirs */
    unsigned short int s_padding1;
    /*
     * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
     */
    unsigned char s_journal_uuid[16]; /* uuid of journal superblock */
    unsigned int s_journal_inum; /* inode number of journal file */
    unsigned int s_journal_dev; /* device number of journal file */
    unsigned int s_last_orphan; /* start of list of inodes to delete */
    unsigned int s_hash_seed[4]; /* HTREE hash seed */
    unsigned char s_def_hash_version; /* Default hash version to use */
    unsigned char s_reserved_char_pad;
    unsigned short int s_reserved_word_pad;
    int s_default_mount_opts;
    int s_first_meta_bg; /* First metablock block group */
    unsigned int s_reserved[190]; /* Padding to the end of the block */
};

/*struct vdiMasterBootRecord
{
    int codeArea[446];
    int partitionTable[64];
    int MBRSignature[2];
};*/

class vdiFile
{
    int fd;
    int *map;
    int cursor;



    public:
        vdiHeader header;
        ext2_super_block SB;
        MasterBootRecord MBR;
        int vdi_open(const char*);
        void vdi_close();
        ssize_t vdi_read(/*int,*/ void*, size_t);
        ssize_t vdi_write(int, const void*, size_t);
        off_t vdi_lseek(off_t, int);
        void setHeader();
        int translate();
        //void convert32(uint32_t);


};

class blockClass
{
    int blockSize;
    int freeSpace;
    int usedSpace;

    public:
        void fetchSuperblock(vdiFile*);
        void fetchInode(vdiFile*);
        void calculateSpace(ext2_super_block*);
        int getFreeSpace();
        int getUsedSpace();
};

int main()
{
    //fstream fs;
    //fs.open ("Test-fixed-1k.vdi", fstream::in);

    //cout << "FILE CONTENTS:  " << fs.read();
    //fs.close

    vdiFile testFile;

    //vdiHeader header;
    testFile.vdi_open("D:/Google Drive/Dropbox/School/Junior Year/Spring/OS Project/Test-fixed-1k.vdi");
    testFile.vdi_read(&testFile.MBR, 512);
    char data[testFile.header.blockSize];
    //off_t a = 1024;
    //testFile.vdi_lseek(512, SEEK_SET);
    //testFile.vdi_read(&testFile.SB, 1024);
    testFile.vdi_lseek((testFile.MBR.partitionTable[0].firstSector*512)+1024, SEEK_SET);

    testFile.vdi_read(&testFile.SB, 1024);

    //cout << "s_inodes count: " << testFile.SB.s_inodes_count << endl;
    //cout << "s_blocks_count: " << testFile.SB.s_blocks_count << endl;
    //cout << testFile.SB.s_r_blocks_count << endl;
    cout << endl;
    cout << "Superblock Magic Number: " << hex << testFile.SB.s_magic << endl;
    cout << endl;
    //testFile.vdi_lseek(0, SEEK_SET);
    //testFile.vdi_read(&header, 164);
    //testFile.vdi_close();
    cout << "Name: " << testFile.header.name << endl;
    cout << "Magic Number: " << hex << testFile.header.magicNumber << dec << endl;
    cout << "header Size: " << testFile.header.headerSize << endl;
    cout << "Image Description: " << testFile.header.imageDescription << endl;
    cout << "Offset Blocks in bytes: " << testFile.header.offsetBlocks << endl;
    cout << "Offset Blocks in MB: " << testFile.header.offsetBlocks/1024 << endl;
    cout << "Offset Data in bytes: " << testFile.header.offsetData << endl;
    cout << "Offset Data in MB: " << (testFile.header.offsetData/1024)/1024 << endl;
    cout << "Sectors: " << testFile.header.Sectors << endl;
    cout << "Sector Size: " << testFile.header.sectorSize << endl;
    cout << "Disk Size in bytes: " << testFile.header.diskSize << endl;
    cout << "Disk Size in MB: " << (testFile.header.diskSize/1024)/1024 << endl;
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

/*
void  vdiFile::convert32 (uint32_t num) {
    uint32_t b0,b1,b2,b3;
    uint32_t res;

    b0 = (num & 0x000000ff) << 24u;
    b1 = (num & 0x0000ff00) << 8u;
    b2 = (num & 0x00ff0000) >> 8u;
    b3 = (num & 0xff000000) >> 24u;

    num = b0 | b1 | b2 | b3;

}
*/
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
    lseek(fd, header.offsetBlocks, SEEK_SET);
    read(fd, map, 4*header.blocksInHDD);
    cout << "MapZero: " << "0" << " = " << map[0] << endl;
    for (int i=0; i < header.blocksInHDD; i ++)
        {
         cout << "Map: " << i << " = " << map[i] << endl;
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
    //fd = file descriptor, read this.
    //*buf = buffer what is read goes here.
    //count = how far to read into file.
    //info = read(fd)
    //fd = open(*fn /*, O_RDONLY*/);
    //lseek the starting point
    //translate();
    //lseek(fd, header.offsetData, SEEK_SET);
    //read(fd, buf, count);
    cursor = translate();
    lseek(fd, cursor, SEEK_SET);
    int poo = read(fd, buf, count);
    if(poo != count)
        cout << "Something went wrong, buckaroo."<<endl;
    else
        cout << "Read went well, buckaroo!" << endl;
}

ssize_t vdiFile::vdi_write(int fd, const void *buf, size_t count)
{
    //write(fd)
    //write(fd, *buf, count);
}

off_t vdiFile::vdi_lseek(off_t offset, int whence)
{
    if (whence == SEEK_SET) {
        cursor = offset;
    }
    else if (whence == SEEK_CUR) {
        cursor += offset;
    }
    else {
        cursor = header.diskSize + offset;
    }

    lseek(fd, offset, whence);
}

void vdiFile::setHeader()
{
    //header.offsetBlocks = 0;
}

int vdiFile::translate()
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
    return location;
    //lseek(fd, location, SEEK_SET);
}
