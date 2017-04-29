#include "vdiUnixCalls.h"
#include "EXT2Functions.h"
#include "boot.h"
#include "datatypes.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <bitset>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

struct vdiHeader {
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

struct ext2_group_desc {
	int	bg_block_bitmap;		/* Blocks bitmap block */
	int	bg_inode_bitmap;		/* Inodes bitmap block */
	int	bg_inode_table;		/* Inodes table block */
	short int	bg_free_blocks_count;	/* Free blocks count */
	short int	bg_free_inodes_count;	/* Free inodes count */
	short int	bg_used_dirs_count;	/* Directories count */
	short int	bg_pad;
	int	bg_reserved[3];
};

struct ext2_inode {
	short int	i_mode;		/* File mode */
	short int	i_uid;		/* Low 16 bits of Owner Uid */
	int	i_size;		/* Size in bytes */
	int	i_atime;	/* Access time */
	int	i_ctime;	/* Creation time */
	int	i_mtime;	/* Modification time */
	int	i_dtime;	/* Deletion Time */
	short int	i_gid;		/* Low 16 bits of Group Id */
	short int	i_links_count;	/* Links count */
	int	i_blocks;	/* Blocks count */
	int	i_flags;	/* File flags */
	union {
		struct {
			int  l_i_reserved1;
		} linux1;
		struct {
			int  h_i_translator;
		} hurd1;
		struct {
			int  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	int	i_block[15];/* Pointers to blocks */
	int	i_generation;	/* File version (for NFS) */
	int	i_file_acl;	/* File ACL */
	int	i_dir_acl;	/* Directory ACL */
	int	i_faddr;	/* Fragment address */
	union {
		struct {
			unsigned char	l_i_frag;	/* Fragment number */
			unsigned char	l_i_fsize;	/* Fragment size */
			unsigned short int	i_pad1;
			short int	l_i_uid_high;	/* these 2 fields    */
			short int	l_i_gid_high;	/* were reserved2[0] */
			unsigned int	l_i_reserved2;
		} linux2;
		struct {
			unsigned char	h_i_frag;	/* Fragment number */
			unsigned char	h_i_fsize;	/* Fragment size */
			short int	h_i_mode_high;
			short int	h_i_uid_high;
			short int	h_i_gid_high;
			int	h_i_author;
		} hurd2;
		struct {
			unsigned char	m_i_frag;	/* Fragment number */
			unsigned char	m_i_fsize;	/* Fragment size */
			unsigned short int	m_pad1;
			unsigned int	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
};

struct ext2_dir_entry_2 {
	int	inode;			/* Inode number */
	short int rec_len;		/* Directory entry length */
	unsigned char name_len;		/* Name length */
	unsigned char file_type;
	char name[];			/* File name, up to EXT2_NAME_LEN */
};

class vdiFile {
    int fd;
    int *map;
    int cursor;



    public:
        vdiHeader header;
        ext2_super_block superBlock;
        ext2_group_desc *blockGroupDescriptorTable;
        ext2_inode* block;
        MasterBootRecord MBR;
        int vdi_open(const char*);
        void vdi_close();
        ssize_t vdi_read(/*int,*/ void*, size_t);
        ssize_t vdi_write(int, const void*, size_t);
        off_t vdi_lseek(off_t, int);
        void setHeader();
        void translate();
};

class blockClass {
	
	private:
		vdiFile* file;
		int blockSize;
		int freeSpace;
		int usedSpace;
		int partitionStart;
		int numBlockGroups;
		int addressesPerBlock;
		int inodesPerBlock;
		int groupDescriptorsPerBlock;
		int groupDescriptorTableBlocksUsed;
		int maxBlocks; 
		int numGDTBlocks;
	
    public:
		blockClass(vdiFile*);
		void doInode(int);
		void doInode2(int);
		void setNumBlockGroups();
		int getNumBlockGroups();
		void setBGDT();
		void getPartitionStart();
        void fetchSuperblock();
        int getBlockSize();
        int getAPB();
        void fetchBlockGroupDescriptorTable();
        //void fetchBlock(int bN, vdiFile*, void* buf);4
        int fetchBlock(int, unsigned char*);
        //void fetchBlock(int, ext2_inode*);
        ext2_inode fetchInode(int);
        //ext2_inode fetchInode(int);
        void calculateSpace(ext2_super_block*);
        void printBlock();
        //void readBitmap(vdiFile*, char, int);
        int getFreeSpace();
        int getUsedSpace();
        void fetchIBM(unsigned char*);
        void fetchBBM(unsigned char*);
        void newFIB(unsigned char*);
        void newFBB(unsigned char*);
};

class directory {
	
	int dirCursor;
	vdiFile* file;
	blockClass* bClass;
	ext2_inode foo;
	unsigned char* buf;
	//int inode;
	//string name;
	
	public:
		directory(vdiFile*, blockClass*);
		int getCursor();
		int getI_Size();
		unsigned char* fetchBlockFromInode(int, ext2_inode);
		void openDir(int);
		int readDir(int&, char*, int&);
		void closeDir();
		void rewindDir();
		//void traverseDirectory(int);
		void fetchBlockFromFile(int, int*, unsigned char*);
		void obtainInodeBlockNums(int, int*);
		
};

bool touched30481 = false;

void setBit(int b, unsigned char* inputMap) {
	int i = b/8;
	int j = b%8;
	//if(touched30481 == true)	
	//	cout << "inputMap[i]: " << bitset<8>((int)inputMap[i]).to_string() << endl;
	inputMap[i] |= (1<<j);
	//if(touched30481 == true) {
	//	cout << "b: " << b << endl;
	//	cout << "i: " << i << endl;
	//	cout << "j: " << j << endl;
	//	cout << "inputMap[i]: " << bitset<8>((int)inputMap[i]).to_string() << endl;
	//}	
}

int countSetBits(unsigned char inputByte) {
	bitset<8> b((int)inputByte);
	return b.count();
}


void traverseDirectory(int, vdiFile*, blockClass*);

unsigned char* nzInBit;
unsigned char* nzBlkBit;


unsigned char* hIB;
unsigned char* hBB;
int hIBCount = 0;
int cIBCount = 0;
int hBBCount = 0;
int cBBCount = 0;
vector <int> stuff;

int main() {

    vdiFile testFile;
    blockClass blockLayer(&testFile);

    testFile.vdi_open("D:/Google Drive/Dropbox/School/Junior Year/Spring/OS Project/Test-fixed-1k.vdi");
    testFile.vdi_read(&testFile.MBR, 512);
    blockLayer.getPartitionStart();
    blockLayer.fetchSuperblock();
    blockLayer.fetchBlockGroupDescriptorTable();
    	
	nzInBit = new unsigned char[testFile.superBlock.s_inodes_count];
	for(int i = 0; i < 10; i++) {
		nzInBit[i] = (unsigned char)1;
	}
	for(int i = 10; i < testFile.superBlock.s_inodes_count; i++) {
		nzInBit[i] = (unsigned char)0;
	}
	
	nzBlkBit = new unsigned char[testFile.superBlock.s_blocks_count];
	for(int i = 0; i < testFile.superBlock.s_blocks_count; i++) {
		nzBlkBit[i] = (unsigned char)0;
	}
	
	
	
	
	
	
	unsigned char* cIB = new unsigned char[testFile.superBlock.s_inodes_count/8];
	for(int i = 0; i < testFile.superBlock.s_inodes_count/8; i++) {
		cIB[i] = (unsigned char)0;
	}
	blockLayer.newFIB(cIB);
	
	hIB = new unsigned char[testFile.superBlock.s_inodes_count/8];
	for(int i = 0; i < testFile.superBlock.s_inodes_count/8; i++) {
		hIB[i] = (unsigned char)0;
	}
	for(int i = 0; i < 10; i++) {
		setBit(i, hIB);
	}	

	unsigned char* cBB = new unsigned char[testFile.superBlock.s_blocks_count/8];
	for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
		cBB[i] = (unsigned char)0;
	}
	blockLayer.newFBB(cBB);

	hBB = new unsigned char[testFile.superBlock.s_blocks_count/8];
	for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
		hBB[i] = (unsigned char)0;
	}
	
	traverseDirectory(2, &testFile, &blockLayer);
	
	directory outsideDir(&testFile, &blockLayer);
		ext2_inode outsideInode = blockLayer.fetchInode(2);
		outsideDir.obtainInodeBlockNums(outsideInode.i_size, outsideInode.i_block);
		outsideInode = blockLayer.fetchInode(7);
		outsideDir.obtainInodeBlockNums(outsideInode.i_size, outsideInode.i_block);
		outsideInode = blockLayer.fetchInode(30480);
		outsideDir.obtainInodeBlockNums(outsideInode.i_size, outsideInode.i_block);

	
	cout << endl << endl;
	for(int i = 0; i < testFile.superBlock.s_inodes_count/8; i++) {
		if(hIB[i] != cIB[i]) {
			cout << "---------- DIFFERENCE AT INDEX (" << i << ") ----------" << endl;
			string humDiff = bitset<8>(hIB[i]).to_string();
			cout << endl << "hIB had: " << humDiff << endl;
			string comDiff = bitset<8>(cIB[i]).to_string();
			cout << "cIB had: " << comDiff << endl << endl << endl;
		}
	}
	
	for(int i = 0; i < testFile.superBlock.s_inodes_count/8; i++) {
		cIBCount += countSetBits(cIB[i]);
	}

	for(int i = 0; i < testFile.superBlock.s_inodes_count/8; i++) {
		hIBCount += countSetBits(hIB[i]);
	}

	for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
		cBBCount += countSetBits(cBB[i]);
	}

	for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
		hBBCount += countSetBits(hBB[i]);
	}	
	
	cout << endl << endl << "  Number of used INODES in ours: " << hIBCount << endl;
	cout << "Number of used INODES in file's: " << cIBCount << endl;
	
	cout << endl << endl << "  Number of used BLOCKS in ours: " << hBBCount << endl;
	cout << "Number of used BLOCKS in file's: " << cBBCount << endl;
	cout << "DIFFERENCE: " << hBBCount - cBBCount << endl;
	
	cout << endl << endl;
	int wrongCount1 = 0;
	int wrongCount2 = 0;
	for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
			string tempCB = bitset<8>(cBB[i]).to_string();
			string tempHB = bitset<8>(hBB[i]).to_string();
		for(int j = 0; j < 8; j++) {	
			if(tempCB.at(7-j) == '1' && tempHB.at(7-j) == '0') { //(tempCB.at(j) == '1' && tempHB.at(j) == '0')) {
				//cout << i << ", " << j << " | ";
				//cout << (i*8)+j+1 << "   |   ";
				wrongCount1++;
			}
			else if(tempCB.at(j) == '0' && tempHB.at(j) == '1') {
				wrongCount2++;
			}
		}
			//cout << "---------- DIFFERENCE AT INDEX (" << i << ") ----------" << endl;
			//string humDiff = bitset<8>(hBB[i]).to_string();
			//cout << endl << "hBB had: " << humDiff << endl;
			//string comDiff = bitset<8>(cBB[i]).to_string();
			//cout << "cBB had: " << comDiff << endl << endl << endl;
		//}
	}
	
	//cout << endl << endl << "hBB: " << bitset<8>(hBB[130040/8]).to_string() << endl << endl;
	//cout << endl << endl << "cBB: " << bitset<8>(cBB[130040/8]).to_string() << endl << endl;
		
	cout << endl << endl << "Listed differences: " << wrongCount1 << endl;
	//cout << "Listed differences2: " << wrongCount2 << endl;
	cout << "Numerical difference: " << hBBCount - cBBCount << endl;
	
	for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
		if(hBB[i] != cBB[i]) {
			cout << endl << endl << "----------- BYTE " << i << " -----------" << endl;
			cout << "hBB: " << bitset<8>(hBB[i]).to_string() << endl;
			cout << "cBB: " << bitset<8>(cBB[i]).to_string() << endl;
		}
	}
	
	for(int i = 0; i < (int)stuff.size(); i++) {
		cout << stuff.at(i) << ",  ";
	}
	cout << stuff.size() << endl;
	
	//cout << endl << bitset<8>((int)cBB[15390/8]).to_string() << endl;
	/*unsigned char test1 = (int)14;
	string test2 = bitset<8> (test1).to_string();
	cout << endl << endl << test2 << endl << endl;
	setBit(6,&test1);
	test2 = bitset<8> (test1).to_string();
	cout << endl << endl << test2 << endl << endl;*/
	
	
	
	
	
	
	
	//cout << endl << "Number of inodes: " << testFile.superBlock.s_inodes_count << endl;
	
	/*int count = 0; 
	for(int i = 0; i < testFile.superBlock.s_inodes_count; i++) {
		if((int)nzInBit[i] == 1)
			count++;
	}*/
	
	//cout << endl << "Used inodes in bitmap: " << count << endl;
	//cout << "Used inodes according to superBlock: " << 337 << endl << endl;
	    
	/*unsigned char* fsInBit = new unsigned char[testFile.superBlock.s_inodes_count]; 
	for(int i = 0; i < 10; i++) {
		fsInBit[i] = (unsigned char)1;
	}
	for(int i = 10; i < testFile.superBlock.s_inodes_count; i++) {
		fsInBit[i] = (unsigned char)0;
	}
	blockLayer.fetchIBM(fsInBit);
	
	blockLayer.newFIBM(fsInBit);*/
	
	//bool same = true;
	//for(int i = 0; i < testFile.superBlock.s_inodes_count; i++) {
		//if(nzInBit[i] != fsInBit[i])
			//same = false;
	//}
	
	//if(same == true)
		//cout << endl << "-------------- INODE BITMAPS ARE THE SAME --------------";
	//else
		//cout << endl << "-------------- :( --------------";	
	
	//unsigned char* fsBlkBit = new unsigned char[testFile.superBlock.s_blocks_count];
	//blockLayer.fetchBBM(fsBlkBit);
	
	//int numUnused = 0; 
	//int numUsed = 0;
	//int numOverused = 0;
	//int ovLoc = 0;
	/*for(int i = 0; i < testFile.superBlock.s_blocks_count; i++) {
		if((int)nzBlkBit[i] == 0) {
			numUnused++;
		}
		else if((int)nzBlkBit[i] == 1) {
			numUsed++;
		}
		else if((int)nzBlkBit[i] > 1) {
			//ovLoc = i;
			numOverused++;
		}
	}*/
	/*
	cout << endl << "number reserved: " << testFile.superBlock.s_r_blocks_count << endl;
	cout << endl << endl << "File system's number of UNUSED files: 21487 = " << numUnused - testFile.superBlock.s_r_blocks_count << endl;
	cout << "File system's amount of OVERUSED: 0 = " << numOverused << " at location " << ovLoc << "; " << (int)nzBlkBit[ovLoc] << " times" << endl;
	cout << "File System's USED files: 108561 = " << testFile.superBlock.s_blocks_count- numUnused - numOverused + testFile.superBlock.s_r_blocks_count << endl;
	cout << endl << "Error is " << abs(testFile.superBlock.s_free_blocks_count-numOverused-numUnused+testFile.superBlock.s_r_blocks_count) << endl;
    
    cout << "\n\n\n\n\n\n";
    vector<int> errs;
	int vCount = 0;
    for(int i = 0; i < testFile.superBlock.s_r_blocks_count; i++) {
		if(nzBlkBit[i] != fsBlkBit[i]) {
			cout << i+1 << " || ";
			vCount++;
		}		
	}
	
	cout << endl << "number printed: " << vCount;*/
	
	
    
    /*cout << "Number of block groups: " << blockLayer.getNumBlockGroups() << "\n" << endl;
	blockLayer.fetchBlock(3, &testFile);
	
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
	for (unsigned int i = 0; i < sizeof(testFile.MBR.unused0); i ++)
	{
		//cout << "Code from Read: " << testFile.MBR.unused0[i] << endl;
	}
	for (int i = 0; i < 4; i ++)
	{
		cout << "Partition Table Type: " << testFile.MBR.partitionTable[i].type << endl;
		cout << "Partition Table: " << testFile.MBR.partitionTable[i].firstSector << endl;
	}
	blockLayer.printBlock(&testFile);
	//cout << endl;
	//cout << "First inode: " << testFile.superBlock.s_first_ino << endl;
	*/
        
        return 0;
}

directory::directory(vdiFile* inputFile, blockClass* bc) {
	file = inputFile;
	bClass = bc;
}

blockClass::blockClass(vdiFile* inputFile) {
	file = inputFile;
}

int vdiFile::vdi_open(const char *pathname)	{
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
    //cout << "MapZero: " << "0" << " = " << map[0] << endl;
    /*for (unsigned int i=0; i < header.blocksInHDD; i ++)
        {
         cout << "Map: " << i << " = " << map[i] << endl;
        }
    */
    //cout << "Name2: " << header.name << endl;
    //translate();
    cursor = 0;
    return 0;
}

void vdiFile::vdi_close() {
    close(fd);
}

ssize_t vdiFile::vdi_read(void *buf, size_t count) {
    translate();
    //cursor = translate();
    //lseek(fd, cursor, SEEK_SET);
    unsigned int check = read(fd, buf, count);
    if(check != count) {
        //cout << "Something went wrong, buckaroo."<<endl;
        return 0;
	}
    else {
        //cout << "Read went well, buckaroo!" << endl;
		return check;
	}
}

off_t vdiFile::vdi_lseek(off_t offset, int whence) {
    if (whence == SEEK_SET) {
        cursor = offset;
    }
    else if (whence == SEEK_CUR) {
        cursor += offset;
    }
    else {
        cursor = header.diskSize + offset;
    }
	return 0;
    return lseek(fd, offset, whence);
}

void vdiFile::setHeader() {
    //header.offsetBlocks = 0;
}

void vdiFile::translate() {
    //int vdiPageSize;
    int pageNumber;
    int offset;
    //int frame;
    int location;

    //cout << "Block Size: " << header.blockSize << endl;
    pageNumber = cursor / header.blockSize;
    //cout << "page number: " << pageNumber << endl;
    offset = cursor % header.blockSize;
    //cout << "Offset: " << offset << endl;
    location = header.offsetData + (map[pageNumber] * header.blockSize) + offset;
    //cout << "location: " << location << endl;
    //return location;
    lseek(fd, location, SEEK_SET);
}

int blockClass::getBlockSize() {
	return blockSize;
}

int blockClass::getAPB() {
	return addressesPerBlock;
}

void blockClass::doInode(int i) {
    //ext2_inode t = fetchInode(i);
    /*cout << "inode number: " << i << endl << endl;
	cout << "inode's size at inode " << i << ": " << t.i_size << endl;
    cout << "inode blocks: " << t.i_blocks << endl;
    cout << "inode links: " << t.i_links_count << endl;*/
}

int blockClass::getNumBlockGroups() {
	return numBlockGroups;
}

void blockClass::setNumBlockGroups() {
	numBlockGroups = (file->superBlock.s_blocks_count+file->superBlock.s_blocks_per_group -  1)/file->superBlock.s_blocks_per_group;
}

void blockClass::setBGDT() {
	file->vdi_lseek((1+file->superBlock.s_first_data_block)*blockSize+partitionStart, SEEK_SET);
	ssize_t test = numBlockGroups * sizeof(struct ext2_group_desc);
	//cout << "\ntest = " << test << endl;
	//cout << "vdi_read = " << file->vdi_read(&file->blockGroupDescriptorTable,test) << endl;
	
	if(file->vdi_read(&file->blockGroupDescriptorTable,test) != test) {
		//cout << "\nfail\n" << endl;
	}
	else {
		//cout << "\n**BGDT SUCCESS**\n" << endl;
	}
	
	//cout << "header block size: " << file->header.blockSize/1048576 << "MB" << endl << endl;
}

void blockClass::getPartitionStart() {
    int partitionTableIndex;
    for (int i = 0; i < 4; i++)
    {
        if (file->MBR.partitionTable[i].type == 0x83)
        {
            partitionTableIndex = i;
        }
    }
    partitionStart = file->MBR.partitionTable[partitionTableIndex].firstSector*512;	
}

void blockClass::fetchSuperblock() {
    file->vdi_lseek(partitionStart+1024, SEEK_SET);
    file->vdi_read(&file->superBlock, sizeof(file->superBlock));
    blockSize = 1024 << file->superBlock.s_log_block_size;
    calculateSpace(&file->superBlock);
    //cout << "blockSize **** " << blockSize << "\n" << endl;
    setNumBlockGroups();
    setBGDT();
    
    addressesPerBlock = blockSize / sizeof(int);
    inodesPerBlock = blockSize / file->superBlock.s_inode_size;
    //cout << "addrPerBlock: " << addressesPerBlock << endl;
    //cout << "inodesPerBlock: " << inodesPerBlock << "\n" << endl;
	
	groupDescriptorsPerBlock = blockSize / sizeof(struct ext2_group_desc);
	groupDescriptorTableBlocksUsed = (numBlockGroups + groupDescriptorsPerBlock - 1) / groupDescriptorsPerBlock;
	maxBlocks = 0x003fffff;
    if(maxBlocks > file->superBlock.s_blocks_count) {
		maxBlocks = file->superBlock.s_blocks_count;
	}
	maxBlocks <<= 10;
	maxBlocks = (maxBlocks + file->superBlock.s_blocks_per_group-1-file->superBlock.s_first_data_block) 
		/ file->superBlock.s_blocks_per_group;	
	numGDTBlocks = ((maxBlocks + groupDescriptorsPerBlock - 1)/groupDescriptorsPerBlock) - groupDescriptorTableBlocksUsed;
	if(numGDTBlocks > addressesPerBlock) {
		numGDTBlocks = addressesPerBlock;
	}
	numGDTBlocks += groupDescriptorTableBlocksUsed;
	//cout << "maxBlocks = " << maxBlocks	 << "\n" << endl;
	//cout << "numGDTBlocks = " << numGDTBlocks << "\n" << endl;
}

void blockClass::fetchBlockGroupDescriptorTable() {
	file->vdi_lseek((1+file->superBlock.s_first_data_block)*blockSize+partitionStart, SEEK_SET);
	file->blockGroupDescriptorTable=new ext2_group_desc[numBlockGroups];
	for(int i = 0; i < numBlockGroups; i++) {
		file->vdi_read(&file->blockGroupDescriptorTable[i],sizeof(file->blockGroupDescriptorTable[i]));
		file->vdi_lseek(sizeof(file->blockGroupDescriptorTable[i]),SEEK_CUR);
	} 
	/*for(int i = 0; i < numBlockGroups; i++) {
		cout << "Group: " << i+1 << endl << endl;
		cout << "BG Block bitmap: " << file->blockGroupDescriptorTable[i].bg_block_bitmap << endl;
		cout << "BG Inode Bitmap: " << file->blockGroupDescriptorTable[i].bg_inode_bitmap << endl;
		cout << "BG Inode Table: " << file->blockGroupDescriptorTable[i].bg_inode_table << endl;
		cout << "BG Free Blocks Count: "<< file->blockGroupDescriptorTable[i].bg_free_blocks_count << endl;
		cout << "BG Free iNodes Count: " << file->blockGroupDescriptorTable[i].bg_free_inodes_count << endl;
		cout << endl << endl;
		//malloc(2000);
	}*/
}

int blockClass::fetchBlock(int b, unsigned char *buf) {
	file->vdi_lseek((b*blockSize)+partitionStart, SEEK_SET);
	return file->vdi_read(buf,blockSize);
}

ext2_inode blockClass::fetchInode(int i) {
	ext2_inode tmp;
	i--;
	int g = (i/file->superBlock.s_inodes_per_group);
	i = (i % file->superBlock.s_inodes_per_group);
	int s = (file->blockGroupDescriptorTable[g].bg_inode_table);
	int b = s + (i/inodesPerBlock);
	i = i % inodesPerBlock;
	ext2_inode* block = new ext2_inode[inodesPerBlock];
	unsigned char* buf = new unsigned char[blockSize];
	fetchBlock(b,buf);
	block = (ext2_inode*)buf;
	tmp = block[i];
	//cout << "fetchInode i_size at index " << i << " is " << block[i].i_size << endl;
	delete[] buf;
	return tmp;
}

void blockClass::printBlock() {
	cout << "Signature: " << hex << file->MBR.magic << dec << endl;
	cout << endl;
	cout << "File System Size: " << file->header.diskSize << endl;
	cout << endl;
	cout << "Space Available:  " << freeSpace << endl;
	cout << endl;
	cout << "Used Space:       " << usedSpace << endl;
	cout << endl;
	cout << "Number of Inodes: " << file->superBlock.s_inodes_count << endl;
	cout << endl;
	cout << "FileSystem State: " << file->superBlock.s_state << endl;	
}

void blockClass::calculateSpace(ext2_super_block* sBlock) {
    int space = (sBlock->s_blocks_count * blockSize);
    freeSpace = ((sBlock->s_free_blocks_count + sBlock->s_r_blocks_count) * blockSize);
    usedSpace = (space - freeSpace);
}

int blockClass::getFreeSpace() {
    return freeSpace;
}

int blockClass::getUsedSpace() {
    return usedSpace;
}

void blockClass::fetchIBM(unsigned char* insteadBuffer) {
	//unsigned char* tempBM = new unsigned char[file->superBlock.s_inodes_count];
	/*unsigned char* tempStuff = new unsigned char[blockSize];
	fetchBlock(260,tempStuff);
	int a = 5;
	cout << "should be 101 " << bitset<3>(a) << endl;
	cout << "Should be 11111111: " << bitset<8>(tempStuff[0]) << endl;
	cout << "Should be 1111----: " << bitset<8>(tempStuff[1]) << endl;
	cout << "Should be 00000000: " << bitset<8>(tempStuff[blockSize-1]) << endl;*/
	
	
	
	//int b = file->blockGroupDescriptorTable[0].bg_inode_bitmap;
	unsigned char* bitmapBlock = new unsigned char[blockSize];
	//fetchBlock(b, bitmapBlock);	
	string tempString;
	unsigned char tempByte;
	int b;
	//b = b = file->blockGroupDescriptorTable[0].bg_inode_bitmap;
	//fetchBlock(b, bitmapBlock);
	for(int m = 0; m < numBlockGroups; m++) {
		b = file->blockGroupDescriptorTable[m].bg_inode_bitmap;
		//cout << "b = " << b << endl; //group 0 --> b = 260
		fetchBlock(b, bitmapBlock); // bitmapBlock = block 260
		for(int i = 0; i < file->superBlock.s_inodes_per_group/8; i++) { // looking at block i
			tempByte = bitmapBlock[i];
			tempString = bitset<8>(tempByte).to_string();
				//cout << endl;
			for(int j = 7; j >= 0; j--) {
				insteadBuffer[m*(file->superBlock.s_inodes_per_group/8)*8 + (i*8)+(7-j)] = ((unsigned char)tempString.at(j)-48);
				//cout << i*sizeof(unsigned char) << " ";
				//cout << (i*sizeof(unsigned char))+(7-j) << " "; //<< ": " <<(int)insteadBuffer[(i*sizeof(unsigned char))+(7-j)] << "  |  ";
				//cout << tempString.at(j)-48 << " ";
			}
		}
	}
	
	int booBerry = 0; 
	for(int i = 0; i < (file->superBlock.s_inodes_per_group/8)*8*numBlockGroups; i++) {
		if((int)insteadBuffer[i] == 0)
			booBerry++;
	}
	//cout << "Number of free inodes: 32175 = " << booBerry << endl;
}

bool bitIsSet(int b, unsigned char* inputMap) {
	int i = b/8;
	int j = b%8;
	if((inputMap[i]&(1<<j)) != 0)
		return true;
	else
		return false;
}

/*void printByte(int bit, unsigned char* inputMap) {
	for(int i = 7; i >= 0; i--) {
		if(bitIsSet(i, inputMap) == true)
			cout << "1";
		else
			cout << "0";
	}
}*/

void blockClass::newFIB(unsigned char* buf) {
	unsigned char* map = new unsigned char[file->superBlock.s_inodes_per_group/8];
	int b;
	for(int m = 0; m < numBlockGroups; m++) {
		b = file->blockGroupDescriptorTable[m].bg_inode_bitmap;
		fetchBlock(b, map);
		for(int i = 0; i < file->superBlock.s_inodes_per_group/8; i++) {
			buf[(m*(file->superBlock.s_inodes_per_group/8))+i] = map[i];
		}
	}
}

void blockClass::newFBB(unsigned char* buf) {
	unsigned char* map = new unsigned char[file->superBlock.s_blocks_per_group];
	int b;
	for(int m = 0; m < numBlockGroups; m++) {
		b = file->blockGroupDescriptorTable[m].bg_block_bitmap;
		fetchBlock(b, map);
		for(int i = 0; i < file->superBlock.s_blocks_per_group/8; i++) {
			buf[(m*(file->superBlock.s_blocks_per_group/8))+i] = map[i];
		}
	}

}
	
	/*for(int i = 7; i >= 0; i--) {
		if(bitIsSet(i, map) == true)
			cout << "1";
		else
			cout << "0";
	}
	cout << endl;
	setBit(0,map);
	for(int i = 7; i >= 0; i--) {
		if(bitIsSet(i, map) == true)
			cout << "1";
		else
			cout << "0";
	}
	cout << endl;
	setBit(2, map);
	for(int i = 7; i >= 0; i--) {
		if(bitIsSet(i, map) == true)
			cout << "1";
		else
			cout << "0";
	}		
	*/

void blockClass::fetchBBM(unsigned char* insteadBuffer) {
	unsigned char* bitmapBlock = new unsigned char[blockSize];	
	string tempString;
	unsigned char tempByte;
	int b;
	for(int m = 0; m < numBlockGroups; m++) {
		b = file->blockGroupDescriptorTable[m].bg_block_bitmap;
		fetchBlock(b, bitmapBlock); 
		for(int i = 0; i < file->superBlock.s_blocks_per_group/8; i++) {
			tempByte = bitmapBlock[i];
			tempString = bitset<8>(tempByte).to_string();
			for(int j = 7; j >= 0; j--) {
				insteadBuffer[m*(file->superBlock.s_blocks_per_group/8)*8 + (i*8)+(7-j)] = ((unsigned char)tempString.at(j)-48);
			}
		}
	}
	int countBBM = 0; 
	for(int i = 0; i < (file->superBlock.s_blocks_per_group/8)*8*numBlockGroups; i++) {
		if((int)insteadBuffer[i] == 0)
			countBBM++;
	}
	//cout << endl << endl << endl << endl << "Number of free blocks: 21487 = " << countBBM << endl;
}

void directory::fetchBlockFromFile(int b, int *i_block, unsigned char *dest) {
	int *list = new int[bClass->getAPB()]; 
	unsigned char* temp = new unsigned char[bClass->getBlockSize()];
	if(b < 12) {
		list = i_block;
		goto direct;
	}
	else if(b < 12 + bClass->getAPB()) {
		list = i_block+12;
		b -= 12; 
		goto single;
	}
	else if(b < 12 + (bClass->getAPB()*(1+bClass->getAPB()))) {
		list = i_block + 13;
		b -= 12 + bClass->getAPB();
		goto dub;
	}
	else {
		//nzBlkBit[i_block[14]-1] += (unsigned char)1;
		bClass->fetchBlock(i_block[14], (unsigned char*)list);
		b -= 12 + bClass->getAPB()*(1+bClass->getAPB()); 
	}
	dub: {
		//nzBlkBit[list[b/bClass->getAPB()/bClass->getAPB()]-1] += (unsigned char)1;
		bClass->fetchBlock(list[b/bClass->getAPB()/bClass->getAPB()], temp);
		list = (int*)temp;
		b %= bClass->getAPB()*bClass->getAPB();
	}
	single: {
		//nzBlkBit[list[b/bClass->getAPB()]-1] += (unsigned char)1;
		bClass->fetchBlock(list[b/bClass->getAPB()],temp);
		list = (int*)temp;
		b %= bClass->getAPB();
	}
	direct: {
		//nzBlkBit[list[b]-1] += (unsigned char)1;
		bClass -> fetchBlock(list[b], dest);
	}
}

void directory::obtainInodeBlockNums(int iSize, int* i_block) {
	unsigned char* temp = new unsigned char[bClass->getBlockSize()];
	int b;
	int *list;
	for(int r = 0; r * bClass->getBlockSize() < iSize; r++) {	
		b = r;
		if(b < 12) {
			list = i_block;			
			goto direct;
		}
		else if(b < 12 + bClass->getAPB()) {
			list = i_block+12;
			b -= 12; 
			goto single;
		}
		else if(b < 12 + (bClass->getAPB()*(1+bClass->getAPB()))) {
			list = i_block + 13;
			b -= 12 + bClass->getAPB();
			goto dub;
		}
		else {
			bClass->fetchBlock(i_block[14], (unsigned char*)list);	
			setBit(i_block[14]-1, hBB);
			b -= 12 + bClass->getAPB()*(1+bClass->getAPB()); 
		}
		dub: {
			bClass->fetchBlock(list[b/bClass->getAPB()/bClass->getAPB()], temp);
			setBit(list[b/bClass->getAPB()/bClass->getAPB()]-1,hBB);
			list = (int*)temp;
			b %= bClass->getAPB()*bClass->getAPB();
		}
		single: {
			bClass->fetchBlock(list[b/bClass->getAPB()],temp);
			setBit(list[b/bClass->getAPB()]-1, hBB);
			list = (int*)temp;
			b %= bClass->getAPB();
		}
		direct: {	
			setBit(list[b]-1, hBB);
		}
	}
	delete[] temp;
}

int directory::getCursor() {
	return dirCursor;
}

int directory::getI_Size() {
	return foo.i_size;
}

void directory::openDir(int index) {
	cout << "---------------------- OPENING INODE (" << index << ") ----------------------\n";
	//cout << "index " << index << endl << endl;
	foo = bClass->fetchInode(index);
	//ext2_inode tempE = bClass->fetchInode(30481);
	//cout << "i_size real " << foo.i_size << endl;// << endl;
	//cout << "i_size fake " << tempE.i_size << endl << endl;
	buf = (unsigned char*)malloc(foo.i_size);
	//int est = ((foo.i_size+bClass->getBlockSize()-1)/bClass->getBlockSize());
	//cout << "estimated number of blocks: " << est << endl << endl;
	for(int i = 0 ; i < 15; i++) {
		cout << i << ": " << foo.i_block[i] << endl;
	}
	
	for(int b = 0; b * bClass->getBlockSize() < foo.i_size; b++) { // run for each block
			if(foo.i_block[b] != 0) {
				unsigned char* buffy = new unsigned char[bClass->getBlockSize()];
				fetchBlockFromFile(b, foo.i_block, buffy);
				for(int j = 0; j < bClass->getBlockSize(); j++) {
					buf[j+b*bClass->getBlockSize()] = buffy[j];
				}
			}
	}	
	dirCursor = 0;	 
}

int directory::readDir(int &index_number, char* name, int &ft) {
	if(dirCursor == foo.i_size) {
		return 0;
	}
	else {
		struct ext2_dir_entry_2 *pDent = (struct ext2_dir_entry_2*)(buf+dirCursor);	
		if(pDent->inode != 0) {	
			//cout << endl << "pDent->rec_len = " << pDent->rec_len << endl;
			//cout << "pDent->inode = " << pDent->inode << endl;
			//cout << "*pDent->name = \"";
			/*for(int i = 0; i < pDent->name_len; i++) {
				cout << (char)pDent->name[i];
			}
			cout << "\"" << endl;*/
		}
		for(int i = 0; i < pDent->name_len; i++) {
					name[i] = pDent->name[i];
		}
		name[pDent->name_len] = 0;
		ft = pDent->file_type;
		if(pDent->inode == 0) {
			dirCursor += (int)pDent->rec_len;
			index_number = (int)pDent->inode;
		}
		else {
			//cout << "file_type = " << ft << endl;
			//cout << "old dirCursor = " << dirCursor /*-*buf*/ << endl;
			dirCursor += (int)pDent->rec_len;
			//cout << "new dirCursor = " << dirCursor/*-*buf*/ << endl;
			//cout << endl << "old index = " << index_number << endl;
			index_number = (int)pDent->inode;
			//cout << "new index = " << index_number << endl;
			//cout << "\n\n----------------------------- \n\n";
			//cout << 2 << " " << endl;
		}
		return 1;
	}	
}

void directory::closeDir() {
	//free(buf);
	delete buf;
}
	
void directory::rewindDir() {
	dirCursor = 0;
}

void traverseDirectory(int inodeNum, vdiFile* file, blockClass* bClass) {
	directory d(file, bClass);
	int fileType = 2;
	char tempName[256];
	d.openDir(inodeNum);
	while(d.readDir(inodeNum, tempName, fileType) == 1) {		
		if(inodeNum != 0 && strcmp(tempName,".") && strcmp(tempName,"..")) {
			setBit(inodeNum-1, hIB);
			ext2_inode tempInode = bClass->fetchInode(inodeNum);
			d.obtainInodeBlockNums(tempInode.i_size, tempInode.i_block);
			if(fileType == 2) 
				traverseDirectory(inodeNum, file, bClass);
		}
	}
	d.closeDir();
}
