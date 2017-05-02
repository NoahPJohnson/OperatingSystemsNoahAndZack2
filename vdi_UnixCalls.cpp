#include "vdiUnixCalls.h"
#include "EXT2Functions.h"
#include "boot.h"
#include "datatypes.h"
#include <sys/types.h>
#include <fcntl.h>
#include <cstddef>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <bitset>
#include <string>
#include <math.h>
#include <vector>
#include <algorithm>

using namespace std;

struct vdiHeader {
    char name[64];
    unsigned int magicNumber; 
    unsigned int version;
    unsigned int headerSize;   
    unsigned int imageType;
    unsigned int imageFlags;
    char imageDescription[256]; 
    unsigned int offsetBlocks; 
    unsigned int offsetData;   
    unsigned int Cylinders;
    unsigned int Heads;
    unsigned int Sectors;      
    unsigned int sectorSize;   
    unsigned int unused;
    unsigned long long int diskSize;     
    unsigned int blockSize;    
    unsigned int blockExtraData;
    unsigned int blocksInHDD;
    unsigned int blocksAllocated;
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
		int numberOfFiles;
		int numberOfDirectories;
		int usedSpace;
		int partitionStart;
		int numBlockGroups;
		int addressesPerBlock;
		int inodesPerBlock;
		int groupDescriptorsPerBlock;
		int groupDescriptorTableBlocksUsed;
		int maxBlocks; 
		int numGDTBlocks;
		ext2_super_block* superBlockArray;
		ext2_group_desc **bGDTArray;
		ext2_group_desc *bGDTArrayEntry;
	
    public:
		blockClass(vdiFile*);
		void calculateFiles();
		void getIBDiffs();
		void getBBDiffs();
		bool compareBlockGroupDescriptorTables();
		void displayFileInformation();
		bool compareSuperBlocks();
		void fetchSuperblockCopies();
		void setNumBlockGroups();
		int getNumBlockGroups();
		void getPartitionStart();
        bool fetchSuperblock();
        int getBlockSize();
        int getAPB();
        void fetchBlockGroupDescriptorTable();
        int fetchBlock(int, unsigned char*);
        ext2_inode fetchInode(int);
        void calculateSpace(ext2_super_block*);
        void printBlock();
        int getFreeSpace();
        int getUsedSpace();
        void newFIB(unsigned char*);
        void newFBB(unsigned char*);
};

class directory {
	
	int dirCursor;
	vdiFile* file;
	blockClass* bClass;
	ext2_inode foo;
	unsigned char* buf;
	
	public:
		directory(vdiFile*, blockClass*);
		int getCursor();
		int getI_Size();
		unsigned char* fetchBlockFromInode(int, ext2_inode);
		void openDir(int);
		int readDir(int&, char*, int&);
		void closeDir();
		void rewindDir();
		void fetchBlockFromFile(int, int*, unsigned char*);
		void obtainInodeBlockNums(int, int, int*);
		
};

void setBit(int b, unsigned char* inputMap) {
	int i = b/8;
	int j = b%8;
	inputMap[i] |= (1<<j);
}

int countSetBits(unsigned char inputByte) {
	bitset<8> b((int)inputByte);
	return b.count();
}

int getBit(int b, unsigned char* inputMap) {
	int i = b/8;
	int j = b%8;
	string tempFoo = bitset<8>((int)inputMap[i]).to_string();
	if(tempFoo.at(7-j) == '1') {
		return 1;
	}
	else {
		return 0;
	}
}

bool bitIsSet(int b, unsigned char* inputMap) {
	int i = b/8;
	int j = b%8;
	if((inputMap[i]&(1<<j)) != 0)
		return true;
	else
		return false;
}

void traverseDirectory(int, vdiFile*, blockClass*);


unsigned char* hIB;
unsigned char* hBB;
unsigned char* cIB;
unsigned char* cBB;
int hIBCount = 0;
int cIBCount = 0;
int hBBCount = 0;
int cBBCount = 0;
int dirTotal = 1;
vector<int> sbCopyNums;
vector<int> reachableUnused;
vector<int> unreachableUsed;
vector<int> unusedReferenced;
vector<int> usedUnreferenced;

int main(int argc, char *argv[]) {
	
	string nameString(argv[1]);
	for(int i = 2; i < argc; i++) {
		nameString.append(" ");
		nameString.append(argv[i]);
	}
	if(nameString.at(0) == '(' && nameString.at(nameString.length()-1) == ')') {
		nameString.erase(0,1);
		nameString.erase(nameString.length()-1,1);
	}
	cout << "new name: " << nameString << endl;
	char* tempNameChar = new char[nameString.length()];
	for(int i = 0; i < (int)nameString.length(); i++) {
		tempNameChar[i] = nameString.at(i);
	}
	
	char* nameChar = tempNameChar;
	
    vdiFile testFile;
    blockClass blockLayer(&testFile);

	//testFile.vdi_open("D:/Google Drive/Dropbox/School/Junior Year/Spring/OS Project/Test-fixed-1k.vdi");
    //testFile.vdi_open("D:/Google Drive/Test-fixed-4k.vdi");
    testFile.vdi_open(nameChar);
    testFile.vdi_read(&testFile.MBR, 512);
    blockLayer.getPartitionStart();
    if(blockLayer.fetchSuperblock() == true) {
		blockLayer.fetchSuperblockCopies();
		blockLayer.calculateFiles();
		
	
		
		cIB = new unsigned char[testFile.superBlock.s_inodes_count/8];
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
	
		cBB = new unsigned char[testFile.superBlock.s_blocks_count/8];
		for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
			cBB[i] = (unsigned char)0;
		}
		blockLayer.newFBB(cBB);
	
		hBB = new unsigned char[testFile.superBlock.s_blocks_count/8];
		for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
			hBB[i] = (unsigned char)0;
		}
		
		directory extraDir(&testFile, &blockLayer);
		ext2_inode extraInode;
		for(int i = 1; i < 11; i++) {
				extraInode = blockLayer.fetchInode(i);
				extraDir.obtainInodeBlockNums(i, extraInode.i_size, extraInode.i_block);
		}
		
		traverseDirectory(2, &testFile, &blockLayer);
		
		for(int i = 0; i < blockLayer.getNumBlockGroups(); i++) {
			setBit((int)testFile.blockGroupDescriptorTable[i].bg_block_bitmap-1, hBB);
			setBit((int)testFile.blockGroupDescriptorTable[i].bg_inode_bitmap-1, hBB);
			for(int j = 0; j < testFile.superBlock.s_inodes_per_group/8; j++) {
				setBit((int)testFile.blockGroupDescriptorTable[i].bg_inode_table+j-1, hBB);
			}
		}
		
		for(int i = 0; i < (int)sbCopyNums.size(); i++) {
			setBit((sbCopyNums.at(i)*testFile.superBlock.s_blocks_per_group), hBB);
			setBit((sbCopyNums.at(i)*testFile.superBlock.s_blocks_per_group)+1, hBB);
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
		
		/*cout << endl;	
		
		
		//for(int i = 0; i 
		
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
		
		cout << endl << endl << "  Number of used INODES in ours: " << hIBCount << endl;
		cout << "Number of used INODES in file's: " << cIBCount << endl;
		
		cout << endl << endl << "  Number of used BLOCKS in ours: " << hBBCount << endl;
		cout << "Number of used BLOCKS in file's: " << cBBCount << endl;
		cout << "DIFFERENCE: " << hBBCount - cBBCount << endl;
		vector<int> tempFoo;
		cout << endl << endl;
		
		int wrongCount1 = 0;
		for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
				string tempCB = bitset<8>(cBB[i]).to_string();
				string tempHB = bitset<8>(hBB[i]).to_string();
			for(int j = 0; j < 8; j++) {	
				if(tempCB.at(7-j) == '1' && tempHB.at(7-j) == '0') { 
					wrongCount1++;
					tempFoo.push_back(i*8+j);
				}
			}
			if(tempCB != tempHB) {
				cout << "---------- DIFFERENCE AT INDEX (" << i << ") ----------" << endl;
				string humDiff = bitset<8>(hBB[i]).to_string();
				cout << endl << "hBB had: " << humDiff << endl;
				string comDiff = bitset<8>(cBB[i]).to_string();
				cout << "cBB had: " << comDiff << endl << endl << endl; 
			}
		}*/
		
		/*
		int wrongBC = 0;
		for(int i = 0; i < testFile.superBlock.s_blocks_count/8; i++) {
			if(hBB[i] != cBB[i]) {
				cout << endl << endl << "----------- BYTE " << i << " -----------" << endl;
				cout << "hBB: " << bitset<8>(hBB[i]).to_string() << endl;
				cout << "cBB: " << bitset<8>(cBB[i]).to_string() << endl;
				for(int j = 0; j < 8; j++) {
					if(bitset<8>(cBB[i]).to_string().at(j) != bitset<8>(hBB[i]).to_string().at(j))
						wrongBC++;
				}
			}
		}
		cout << endl << "Number of different bytes above: " << wrongBC << endl;
		
		cout << endl << endl << "Number of times we get a zero when fs gets a 1: " << wrongCount1 << endl;
		cout << "Difference in bit counts: " << hBBCount - cBBCount << endl;
		
		cout << endl << endl << "cBBCount: " << cBBCount << endl;
		
		//cout << endl << "cooCount: " << cooCount << endl;
	        
		int floop = 0;
	    for(int i = 1; i < testFile.superBlock.s_inodes_count+1; i++) {
			ext2_inode foo = blockLayer.fetchInode(i);
			floop += foo.i_blocks;
		}    
		
		//cout << endl << endl << "hmmm: " << floop/2 << endl;
	    cout << endl << endl;
	    for(int i = 0; i < (int)tempFoo.size(); i++) {
			cout << i+1 << ": " << tempFoo.at(i)+1 << endl;
		}  */
		
		blockLayer.displayFileInformation();
		/*for(int i = 0; i < (int)sbCopyNums.size(); i++) {
			cout << (sbCopyNums.at(i)*testFile.superBlock.s_blocks_per_group)+1 << endl;
			cout << (sbCopyNums.at(i)*testFile.superBlock.s_blocks_per_group)+2 << endl;
		}*/
	 }
	 else {
		 cout << "The superblock magic number is wrong!" << endl;
	 }
        testFile.vdi_close();
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
    if (fd < 0) {
        return 1;
    }
    read(fd, &header, sizeof(header));
    map = new int[header.blocksInHDD];
    lseek(fd, header.offsetBlocks, SEEK_SET);
    read(fd, map, 4*header.blocksInHDD);
    cursor = 0;
    return 0;
}

void vdiFile::vdi_close() {
    close(fd);
}

ssize_t vdiFile::vdi_read(void *buf, size_t count) {
    translate();
    unsigned int check = read(fd, buf, count);
    if(check != count) {
        return 0;
	}
    else {
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
	//return 0;
    return lseek(fd, offset, whence);
}

void vdiFile::translate() {
    int pageNumber;
    int offset;
    int location;

    pageNumber = cursor / header.blockSize;
    offset = cursor % header.blockSize;
    location = header.offsetData + (map[pageNumber] * header.blockSize) + offset;
    lseek(fd, location, SEEK_SET);
}

void blockClass::getIBDiffs() {
	bool rU = false;
	bool uU = false;
	for(int i = 0; i < file->superBlock.s_inodes_count/8; i++) {
		if(hIB[i] != cIB[i]) {
			for(int j = 0; j < 8; j++) {
				if(getBit(i*8 + j, hIB) != getBit(i*8 + j,cIB)) {
					if(getBit(i*8 + j, hIB) == 1) {
						reachableUnused.push_back(i*8+j+1);
						rU = true;
					} 
					else if(getBit(i*8 + j, hIB) == 0) {
						unreachableUsed.push_back(i*8+j+1);
						uU = true;
					}
				}
			}
		}
	}
	if(uU == true) {
		cout << "Inode(s) marked as used but not reachable from the root directory: ";
		for(int i = 0; i < (int)unreachableUsed.size(); i++) {
			if(i != (int)unreachableUsed.size()-1)
				cout << unreachableUsed.at(i) << ", ";
			else
				cout << unreachableUsed.at(i) << " " << endl;
		}
	}
	if(rU == true) {
		cout << "A directory entry references an unused inode." << endl;
		cout << "Inode(s) marked as unused but reachable from the root directory: ";
		for(int i = 0; i < (int)reachableUnused.size(); i++) {
			if(i != (int)reachableUnused.size()-1)
				cout << reachableUnused.at(i) << ", ";
			else
				cout << reachableUnused.at(i) << " " << endl;
		}
	}	
	if(uU == false && rU == false) {
		cout << "All inodes marked as used ARE reachable from the root directory." << endl;
		cout << "All inodes marked as unused are NOT reachable from the root directory." << endl;
		cout << "Every directory entry DOES reference an in-use inode." << endl;
	}
}

void blockClass::getBBDiffs() {
	bool uR = false;
	bool uU = false;
	for(int i = 0; i < file->superBlock.s_blocks_count/8; i++) {
		if(hBB[i] != cBB[i]) {
			for(int j = 0; j < 8; j++) {
				if(getBit(i*8 + j, hBB) != getBit(i*8 + j,cBB)) {
					if(getBit(i*8 + j, hBB) == 1) {
						unusedReferenced.push_back(i*8+j+1);
						uR = true;
					} 
					else if(getBit(i*8 + j, hBB) == 0) {
						usedUnreferenced.push_back(i*8+j+1);
						uU = true;
					}
				}
			}
		}
	}
	if(uU == true) {
		cout << "Used blocks not referenced by any inode: ";
		for(int i = 0; i < (int)usedUnreferenced.size(); i++) {
			if(i != (int)usedUnreferenced.size()-1)
				cout << usedUnreferenced.at(i) << ", ";
			else
				cout << usedUnreferenced.at(i) << " " << endl;
		}
	}
	if(uR == true) {
		cout << "Unused data blocks referenced by an inode: ";
		for(int i = 0; i < (int)unusedReferenced.size(); i++) {
			if(i != (int)unusedReferenced.size()-1)
				cout << unusedReferenced.at(i) << ", ";
			else
				cout << unusedReferenced.at(i) << " " << endl;
		}
	}	
	if(uU == false && uR == false) {
		cout << "All used data blocks ARE referenced by at least 1 inode." << endl;
		cout << "NO unused data blocks are referenced by an inode." << endl;
	}
}

int blockClass::getBlockSize() {
	return blockSize;
}

bool blockClass::compareSuperBlocks() {
    bool consistent = true;
    for (int i = 0; i < (int)sbCopyNums.size(); i ++)
    {
         if (file->superBlock.s_inodes_count != superBlockArray[i].s_inodes_count)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Inodes Count: " << superBlockArray[i].s_inodes_count << endl; 
             consistent = false;
             superBlockArray[i].s_inodes_count = file->superBlock.s_inodes_count;
             cout << "CORRECTED TO: " << superBlockArray[i].s_inodes_count << endl;
         }
         if (file->superBlock.s_blocks_count != superBlockArray[i].s_blocks_count)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Blocks Count: " << superBlockArray[i].s_blocks_count << endl; 
             consistent = false;
             superBlockArray[i].s_blocks_count = file->superBlock.s_blocks_count;
             cout << "CORRECTED TO: " << superBlockArray[i].s_blocks_count << endl;
         }
         if (file->superBlock.s_r_blocks_count != superBlockArray[i].s_r_blocks_count)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Reserved Blocks Count: " << superBlockArray[i].s_r_blocks_count << endl; 
             consistent = false;
             superBlockArray[i].s_r_blocks_count = file->superBlock.s_r_blocks_count;
             cout << "CORRECTED TO: " << superBlockArray[i].s_r_blocks_count << endl;
         }
         if (file->superBlock.s_first_data_block != superBlockArray[i].s_first_data_block)
         {
             cout << "Index: " << sbCopyNums.at(i) << " First Data Block: " << superBlockArray[i].s_first_data_block << endl; 
             consistent = false;
             superBlockArray[i].s_first_data_block = file->superBlock.s_first_data_block;
             cout << "CORRECTED TO: " << superBlockArray[i].s_first_data_block << endl;
         }
         if (file->superBlock.s_log_block_size != superBlockArray[i].s_log_block_size)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Block Size: " << superBlockArray[i].s_log_block_size << endl; 
             consistent = false;
             superBlockArray[i].s_log_block_size = file->superBlock.s_log_block_size;
             cout << "CORRECTED TO: " << superBlockArray[i].s_log_block_size << endl;
         }
         if (file->superBlock.s_log_frag_size != superBlockArray[i].s_log_frag_size)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Frag Size: " << superBlockArray[i].s_log_frag_size << endl; 
             consistent = false;
             superBlockArray[i].s_log_frag_size = file->superBlock.s_log_frag_size;
             cout << "CORRECTED TO: " << superBlockArray[i].s_log_frag_size << endl;
         }
         if (file->superBlock.s_blocks_per_group != superBlockArray[i].s_blocks_per_group)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Blocks Per Group: " << superBlockArray[i].s_blocks_per_group << endl; 
             consistent = false;
             superBlockArray[i].s_blocks_per_group = file->superBlock.s_blocks_per_group;
             cout << "CORRECTED TO: " << superBlockArray[i].s_blocks_per_group << endl;
         }
         if (file->superBlock.s_frags_per_group != superBlockArray[i].s_frags_per_group)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Frags Per Group: " << superBlockArray[i].s_frags_per_group << endl; 
             consistent = false;
             superBlockArray[i].s_frags_per_group = file->superBlock.s_frags_per_group;
             cout << "CORRECTED TO: " << superBlockArray[i].s_frags_per_group << endl;
         }
         if (file->superBlock.s_magic != superBlockArray[i].s_magic)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Magic: " << superBlockArray[i].s_magic << endl; 
             consistent = false;
             superBlockArray[i].s_magic = file->superBlock.s_magic;
             cout << "CORRECTED TO: " << superBlockArray[i].s_magic << endl;
         }
         if (file->superBlock.s_minor_rev_level != superBlockArray[i].s_minor_rev_level)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Rev Level: " << superBlockArray[i].s_minor_rev_level << endl; 
             consistent = false;
             superBlockArray[i].s_minor_rev_level = file->superBlock.s_minor_rev_level;
             cout << "CORRECTED TO: " << superBlockArray[i].s_minor_rev_level << endl;
         }
         if (file->superBlock.s_creator_os != superBlockArray[i].s_creator_os)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Creator OS: " << superBlockArray[i].s_creator_os << endl; 
             consistent = false;
             superBlockArray[i].s_creator_os = file->superBlock.s_creator_os;
             cout << "CORRECTED TO: " << superBlockArray[i].s_creator_os << endl;
         }
         if (file->superBlock.s_first_ino != superBlockArray[i].s_first_ino)
         {
             cout << "Index: " << sbCopyNums.at(i) << " First Inode: " << superBlockArray[i].s_first_ino << endl; 
             consistent = false;
             superBlockArray[i].s_first_ino = file->superBlock.s_first_ino;
             cout << "CORRECTED TO: " << superBlockArray[i].s_first_ino << endl;
         }
         if (file->superBlock.s_inode_size != superBlockArray[i].s_inode_size)
         {
             cout << "Index: " << sbCopyNums.at(i) << " Inode Size: " << superBlockArray[i].s_inode_size << endl; 
             consistent = false;
             superBlockArray[i].s_inode_size = file->superBlock.s_inode_size;
             cout << "CORRECTED TO: " << superBlockArray[i].s_inode_size << endl;
         }
    }
    return consistent;
}

int blockClass::getAPB() {
	return addressesPerBlock;
}

int blockClass::getNumBlockGroups() {
	return numBlockGroups;
}

void blockClass::setNumBlockGroups() {
	numBlockGroups = (file->superBlock.s_blocks_count+file->superBlock.s_blocks_per_group -  1)/file->superBlock.s_blocks_per_group;
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

bool blockClass::fetchSuperblock() {
    file->vdi_lseek(partitionStart+1024, SEEK_SET);
    file->vdi_read(&file->superBlock, sizeof(file->superBlock));
    blockSize = 1024 << file->superBlock.s_log_block_size;
    if((int)file->superBlock.s_magic != -4269) { // -4269 = normal file->superBlock.s_magic cast as int 
		return false;
	}
    calculateSpace(&file->superBlock);
    setNumBlockGroups();
    fetchBlockGroupDescriptorTable();
    
    addressesPerBlock = blockSize / sizeof(int);
    inodesPerBlock = blockSize / file->superBlock.s_inode_size;
	
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
	return true;
}

void blockClass::fetchSuperblockCopies() {
	sbCopyNums.push_back(0);
	for(int h = 3; h <= 7; h += 2) {
		for(int i = 0; pow(h,i) < numBlockGroups; i++) {
			bool alreadyIn = false;
			for(int j = 0; j < (int)sbCopyNums.size(); j++) {
				if(sbCopyNums.at(j) == pow(h,i))
					alreadyIn = true;
			}
			if(alreadyIn == false)
				sbCopyNums.push_back(pow(h,i));
		}		
	}
	
	bGDTArray = new ext2_group_desc*[(int)sbCopyNums.size()];
	bGDTArrayEntry = new ext2_group_desc[numBlockGroups];
	for(int i = 0; i < (int)sbCopyNums.size(); i++) {
		file->vdi_lseek((1+file->superBlock.s_first_data_block)*blockSize+partitionStart, SEEK_SET);
		bGDTArray[i] = bGDTArrayEntry;
		for(int j = 0; j < numBlockGroups; j++) {	
			file->vdi_read(&bGDTArrayEntry[j], sizeof(bGDTArrayEntry[j]));
			file->vdi_lseek(sizeof(bGDTArrayEntry[j]), SEEK_CUR);
		}
	} 
	
	/*cout << endl << endl;
	for(int i = 0; i < (int)sbCopyNums.size(); i++) {
		cout << sbCopyNums.at(i) << " ";
	}
	cout << endl << endl;*/
	
	superBlockArray = new ext2_super_block[(int)sbCopyNums.size()];
	for(int i = 0; i < (int)sbCopyNums.size(); i++) {
		file->vdi_lseek(partitionStart+1024+(sbCopyNums.at(i)*blockSize*file->superBlock.s_blocks_per_group), SEEK_SET);
		file->vdi_read(&superBlockArray[i], sizeof(file->superBlock));
	}
}

bool blockClass::compareBlockGroupDescriptorTables() {
    bool consistent = true;
    for (int i = 0; i < (int)sbCopyNums.size(); i ++) {
        for (int j = 0; j < numBlockGroups; j++) {
             if (file->blockGroupDescriptorTable[j].bg_block_bitmap != bGDTArray[i][j].bg_block_bitmap) {
                 cout << "Index: " << i << ", " << j << " Block Bitmap: " << bGDTArray[i][j].bg_block_bitmap << endl;
                 consistent = false;
                 bGDTArray[i][j].bg_block_bitmap = file->blockGroupDescriptorTable[j].bg_block_bitmap;
                 cout << "CORRECTED TO: " << bGDTArray[i][j].bg_block_bitmap << endl;
             }
             if (file->blockGroupDescriptorTable[j].bg_inode_bitmap != bGDTArray[i][j].bg_inode_bitmap) {
                 cout << "Index: " << i << ", " << j << " Inode Bitmap: " << bGDTArray[i][j].bg_inode_bitmap << endl;
                 consistent = false;
                 bGDTArray[i][j].bg_inode_bitmap = file->blockGroupDescriptorTable[j].bg_inode_bitmap;
                 cout << "CORRECTED TO: " << bGDTArray[i][j].bg_inode_bitmap << endl;
             }
             if (file->blockGroupDescriptorTable[j].bg_inode_table != bGDTArray[i][j].bg_inode_table) {
                 cout << "Index: " << i << ", " << j << " Inode Table:  " << bGDTArray[i][j].bg_inode_table << endl;
                 consistent = false;
                 bGDTArray[i][j].bg_inode_table = file->blockGroupDescriptorTable[j].bg_inode_table;
                 cout << "CORRECTED TO: " << bGDTArray[i][j].bg_inode_table << endl;
             }
             if (file->blockGroupDescriptorTable[j].bg_free_blocks_count != bGDTArray[i][j].bg_free_blocks_count) {
                 cout << "Index: " << i << ", " << j << " Free Blocks:  " << bGDTArray[i][j].bg_free_blocks_count << endl;
                 consistent = false;
                 bGDTArray[i][j].bg_free_blocks_count = file->blockGroupDescriptorTable[j].bg_free_blocks_count;
                 cout << "CORRECTED TO: " << bGDTArray[i][j].bg_free_blocks_count << endl;
             }
             if (file->blockGroupDescriptorTable[j].bg_free_inodes_count != bGDTArray[i][j].bg_free_inodes_count) {
                 cout << "Index: " << i << ", " << j << " Free Inodes:  " << bGDTArray[i][j].bg_free_inodes_count << endl;
                 consistent = false;
                 bGDTArray[i][j].bg_free_inodes_count = file->blockGroupDescriptorTable[j].bg_free_inodes_count;
                 cout << "CORRECTED TO: " << bGDTArray[i][j].bg_free_inodes_count << endl;
             }
        }
    }
    return consistent;
}

void blockClass::calculateFiles() {
    numberOfFiles = 0;
    numberOfDirectories = 0;
    for (int i = 0; i < numBlockGroups; i++)
    {
         numberOfDirectories += file->blockGroupDescriptorTable[i].bg_used_dirs_count;
    }
    numberOfFiles = file->superBlock.s_inodes_count - (numberOfDirectories+file->superBlock.s_free_inodes_count);
}

void blockClass::displayFileInformation() { 
cout << "File System Size: " << file->header.diskSize << " bytes" << endl; 
     
    cout << endl; 
    cout << "Space Available:  " << getFreeSpace() << " bytes" << endl; 
     
    cout << endl; 
    cout << "Used Space:       " << getUsedSpace() << " bytes" << endl; 
     
 
    cout << endl; 
    cout << "Number of Blocks: " << " Total - " << file->superBlock.s_blocks_count << " Free - " << file->superBlock.s_free_blocks_count <<  
                                    " Reserved - " << file->superBlock.s_r_blocks_count << endl; 
    cout << "Number of Inodes: " << " Total - " << file->superBlock.s_inodes_count << "  Free - " << file->superBlock.s_free_inodes_count << endl; 
    
    cout << endl;
    cout << "Superblocks consistent? " << endl;
    if (compareSuperBlocks() == true) {
        cout << "--- YES ---" << endl;
    }
    else {
        cout << "--- NO ---" << endl;
    }
    
    cout << endl;
    cout << "Block Group Descriptor Tables consistant? " << endl;
    if (compareBlockGroupDescriptorTables() == true) {
        cout << "--- YES ---" << endl;
    }
    else {
        cout << "--- NO ---" << endl;
    }
       
    cout << endl; 
    cout << "First Data Block: " << file->superBlock.s_first_data_block << endl; 
     
    cout << endl; 
    cout << "Block Size:       " << blockSize << " bytes" << endl; 
 
    cout << endl; 
    cout << "Block Groups:     " << numBlockGroups << endl; 
 
    cout << endl; 
    cout << "Blocks Per Group: " << file->superBlock.s_blocks_per_group << endl; 
     
    cout << endl; 
    cout << "Inodes Per Group: " << file->superBlock.s_inodes_per_group << endl; 
 
    cout << endl; 
    cout << "GDT Blocks      : " << numGDTBlocks << endl; 
 
    cout << endl; 
    cout << "Inodes Per Block: " << inodesPerBlock << endl; 
 
    cout << endl; 
    cout << "Addresses Per Bk: " << addressesPerBlock << endl; 
 
    cout << endl; 
    for (int i = 0; i < numBlockGroups; i++) 
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
	if(numberOfFiles == (hIBCount - dirTotal))
		cout << "The number of existing files reported by the superblock IS correct." << endl;
	else
		cout << "The number of existing files reported by the superblock is NOT correct." << endl;	
 
    cout << endl; 
    cout << "Directories:      " << numberOfDirectories << endl; 
	if(numberOfDirectories == dirTotal)
		cout << "The number of existing directories reported by the superblock IS correct." << endl;
	else
		cout << "The number of existing directories reported by the superblock is NOT correct." << endl;
  
    cout << endl; 
    cout << "FileSystem State: " << file->superBlock.s_state << endl; 
 
    cout << endl; 
    cout << "SuperBlock mag:   " << hex << file->superBlock.s_magic << dec << endl; 
    
    cout << endl;
    getIBDiffs();
    
    cout << endl;
    getBBDiffs();
}

void blockClass::fetchBlockGroupDescriptorTable() {
	file->vdi_lseek((1+file->superBlock.s_first_data_block)*blockSize+partitionStart, SEEK_SET);
	file->blockGroupDescriptorTable=new ext2_group_desc[numBlockGroups];
	for(int i = 0; i < numBlockGroups; i++) {
		file->vdi_read(&file->blockGroupDescriptorTable[i],sizeof(file->blockGroupDescriptorTable[i]));
		file->vdi_lseek(sizeof(file->blockGroupDescriptorTable[i]),SEEK_CUR);
	} 
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

void blockClass::newFIB(unsigned char* buf) {
	unsigned char* map = new unsigned char[blockSize];
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
	unsigned char* map = new unsigned char[blockSize];
	int b;
	for(int m = 0; m < numBlockGroups; m++) {
		b = file->blockGroupDescriptorTable[m].bg_block_bitmap;
		fetchBlock(b, map);
		for(int i = 0; i < file->superBlock.s_blocks_per_group/8; i++) {
			buf[(m*(file->superBlock.s_blocks_per_group/8))+i] = map[i];
		}
	}
}
	
void directory::fetchBlockFromFile(int b, int *i_block, unsigned char *dest) {
	int aPB = bClass->getAPB();
	int *list = new int[aPB]; 
	unsigned char *temp = new unsigned char[bClass->getBlockSize()];
	if(b < 12) {
		list = i_block;
		goto direct;
	}
	else if(b < 12 + aPB) {
		list = i_block+12;
		b -= 12; 
		goto single;
	}
	else if(b < 12 + (aPB*(1+aPB))) {
		list = i_block + 13;
		b -= 12 + aPB;
		goto dub;
	}
	else {
		bClass->fetchBlock(i_block[14], (unsigned char*)list);
		b -= 12 + aPB*(1+aPB); 
	}
	dub: {
		bClass->fetchBlock(list[b/aPB/aPB], temp);
		list = (int*)temp;
		b %= aPB*aPB;
	}
	single: {
		bClass->fetchBlock(list[b/aPB],temp);
		list = (int*)temp;
		b %= aPB;
	}
	direct: {
		bClass -> fetchBlock(list[b], dest);
	}
	delete[] temp;
	//delete[] list;
}

void directory::obtainInodeBlockNums(int inNum, int iSize, int* i_block) {
	int b;
	int *list;
	int lb;
	unsigned char* temp = new unsigned char[bClass->getBlockSize()];
	int aPB = bClass->getAPB();
	for(int r = 0; r * bClass->getBlockSize() < iSize; r++) {	
		b = r;
		if(b < 12) {
			list = i_block;			
			goto direct;
		}
		else if(b < 12 + aPB) {
			list = i_block+12;
			b -= 12; 
			goto single;
		}
		else if(b < 12 + (aPB*(1+aPB))) {
			list = i_block + 13;
			b -= 12 + aPB;
			goto dub;
		}
		else {
			lb = i_block[14];
			if(i_block[14]-1 > 0 && i_block[14] <= file->superBlock.s_blocks_count)	{
				bClass->fetchBlock(i_block[14], (unsigned char*)list);	
				if(getBit(lb-file->superBlock.s_first_data_block, hBB) != 1) {
					setBit(lb-file->superBlock.s_first_data_block, hBB);
				}
			}
			b -= 12 + aPB*(1+aPB); 
		}
		dub: {
			lb = list[b/aPB/aPB];
			if(list[b/aPB/aPB]-1 > 0 && list[b/aPB/aPB] <= file->superBlock.s_blocks_count) {
				bClass->fetchBlock(list[b/aPB/aPB], temp);
				if(getBit(lb-file->superBlock.s_first_data_block, hBB) != 1) {
					setBit(lb-file->superBlock.s_first_data_block,hBB);
				}
			}
			list = (int*)temp;
			b %= aPB*aPB;
		}
		single: {
			lb = list[b/aPB];
			if(list[b/aPB]-1 > 0 && list[b/aPB] <= file->superBlock.s_blocks_count) {	
				bClass->fetchBlock((int)list[b/aPB],temp);
				if(getBit(lb-file->superBlock.s_first_data_block, hBB) != 1) {
					setBit(lb-file->superBlock.s_first_data_block, hBB);
				}
			}	
			list = (int*)temp;
			b %= aPB;
		}
		direct: {	
			lb = list[b];
			if(list[b]-1 > 0 && list[b] <= file->superBlock.s_blocks_count) {
				if(getBit(lb-file->superBlock.s_first_data_block, hBB) == 1) {
					//cout << endl << endl << "ERROR MORE THAN ONE BLOCK TOUCH " << lb-1 << endl << endl;
				}				
				else if(getBit(lb-file->superBlock.s_first_data_block, hBB) == 0) {
					setBit(lb-file->superBlock.s_first_data_block, hBB);
					/*if(inNum == 7) {
						count7++;
						cout << "list[b]-1: " << lb-1 << "; " << count7 << endl;
					}*/
				}
			}
		}
	//delete[] temp;
	}
}

int directory::getCursor() {
	return dirCursor;
}

int directory::getI_Size() {
	return foo.i_size;
}

void directory::openDir(int index) {
	//cout << "---------------------- OPENING INODE (" << index << ") ----------------------\n";
	foo = bClass->fetchInode(index);
	buf = (unsigned char*)malloc(foo.i_size);
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
	delete[] buf;
}
	
void directory::rewindDir() {
	dirCursor = 0;
}

void traverseDirectory(int inodeNum, vdiFile* file, blockClass* bClass) {
	directory d(file, bClass);
	int fileType = 2;
	char tempName[256];
	d.openDir(inodeNum);
	
	//if(fileType != 2) {
	//			fileTotal++;
	//}
	while(d.readDir(inodeNum, tempName, fileType) == 1) {		
		if(inodeNum != 0 && strcmp(tempName,".") && strcmp(tempName,"..")) {
			ext2_inode tempInode = bClass->fetchInode(inodeNum);
			d.obtainInodeBlockNums(inodeNum, tempInode.i_size, tempInode.i_block);
			setBit(inodeNum-1, hIB);
			if(fileType == 2) {
				traverseDirectory(inodeNum, file, bClass); 
				dirTotal++;
			}
		}		
	}
	d.closeDir();
}
