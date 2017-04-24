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
        void fetchBlockGroupDescriptorTable();
        //void fetchBlock(int bN, vdiFile*, void* buf);4
        int fetchBlock2(int, unsigned char*);
        void fetchBlock(int, ext2_inode*);
        ext2_inode fetchInode2(int);
        ext2_inode fetchInode(int);
        void calculateSpace(ext2_super_block*);
        void printBlock();
        //void readBitmap(vdiFile*, char, int);
        int getFreeSpace();
        int getUsedSpace();
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
		unsigned char* fetchBlockFromFile(int, ext2_inode);
		void openDir(int);
		int readDir(int&, char*);
		void closeDir(void);
		void rewindDir(void);
		void traverseDirectory();
		void scanDir(int);
		
};

int main() {

    vdiFile testFile;
    blockClass blockLayer(&testFile);

    testFile.vdi_open("D:/Google Drive/Dropbox/School/Junior Year/Spring/OS Project/Test-fixed-1k.vdi");
    testFile.vdi_read(&testFile.MBR, 512);
    blockLayer.getPartitionStart();
    blockLayer.fetchSuperblock();
    blockLayer.fetchBlockGroupDescriptorTable();
    //blockLayer.doInode(12);
    //cout << endl;
    //blockLayer.doInode2(12);
    directory dir(&testFile, &blockLayer);
    dir.openDir(2);
    int index = 0;
    char name = 'h';
    dir.readDir(index, &name);
    cout << endl << "printing changed index in main: " << (int)index << endl;
    cout << "name: " << name << endl;
    //blockLayer.readBitmap(&testFile, 'b', 3);
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
    cout << "MapZero: " << "0" << " = " << map[0] << endl;
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

void blockClass::doInode(int i) {
    ext2_inode t = fetchInode(i);
    cout << "inode number: " << i << endl << endl;
	cout << "inode's size at inode " << i << ": " << t.i_size << endl;
    cout << "indoe blocks: " << t.i_blocks << endl;
    cout << "inode links: " << t.i_links_count << endl;
}

void blockClass::doInode2(int i) {
    ext2_inode t = fetchInode2(i);
    cout << "inode number: " << i << endl << endl;
	cout << "inode's size at inode " << i << ": " << t.i_size << endl;
    cout << "indoe blocks: " << t.i_blocks << endl;
    cout << "inode links: " << t.i_links_count << endl;
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
		cout << "\nfail\n" << endl;
	}
	else {
		cout << "\n**BGDT SUCCESS**\n" << endl;
	}
	
	cout << "header block size: " << file->header.blockSize/1048576 << "MB" << endl << endl;
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
    cout << "blockSize **** " << blockSize << "\n" << endl;
    setNumBlockGroups();
    setBGDT();
    
    addressesPerBlock = blockSize / sizeof(int);
    inodesPerBlock = blockSize / file->superBlock.s_inode_size;
    cout << "addrPerBlock: " << addressesPerBlock << endl;
    cout << "inodesPerBlock: " << inodesPerBlock << "\n" << endl;
	
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
	cout << "maxBlocks = " << maxBlocks << "\n" << endl;
	cout << "numGDTBlocks = " << numGDTBlocks << "\n" << endl;
}

void blockClass::fetchBlockGroupDescriptorTable() {
	file->vdi_lseek((1+file->superBlock.s_first_data_block)*blockSize+partitionStart, SEEK_SET);
	file->blockGroupDescriptorTable=new ext2_group_desc[numBlockGroups];
	for(int i = 0; i < numBlockGroups; i++) {
		file->vdi_read(&file->blockGroupDescriptorTable[i],sizeof(file->blockGroupDescriptorTable[i]));
		file->vdi_lseek(sizeof(file->blockGroupDescriptorTable[i]),SEEK_CUR);
	} 
	for(int i = 0; i < numBlockGroups; i++) {
		cout << "Group: " << i+1 << endl << endl;
		cout << "BG Block bitmap: " << file->blockGroupDescriptorTable[i].bg_block_bitmap << endl;
		cout << "BG Inode Bitmap: " << file->blockGroupDescriptorTable[i].bg_inode_bitmap << endl;
		cout << "BG Inode Table: " << file->blockGroupDescriptorTable[i].bg_inode_table << endl;
		cout << "BG Free Blocks Count: "<< file->blockGroupDescriptorTable[i].bg_free_blocks_count << endl;
		cout << "BG Free iNodes Count: " << file->blockGroupDescriptorTable[i].bg_free_inodes_count << endl;
		cout << endl << endl;
		malloc(2000);
	}
}

int blockClass::fetchBlock2(int b, unsigned char *buf) {
	file->vdi_lseek((b*blockSize)+partitionStart, SEEK_SET);
	return file->vdi_read(buf,blockSize);
}

void blockClass::fetchBlock(int b, ext2_inode *buf) {
	file->vdi_lseek((b*blockSize)+partitionStart, SEEK_SET);
	for(int i = 0; i < inodesPerBlock; i++) {
		file->vdi_read(&buf[i], sizeof(buf[i]));
		file->vdi_lseek(sizeof(buf[i]), SEEK_CUR);
	}
} 

ext2_inode blockClass::fetchInode2(int i) {
	i--;
	int g = (i/file->superBlock.s_inodes_per_group);
	i = (i % file->superBlock.s_inodes_per_group);
	int s = (file->blockGroupDescriptorTable[g].bg_inode_table);
	int b = s + (i/inodesPerBlock);
	i = i % inodesPerBlock;
	ext2_inode* block = new ext2_inode[inodesPerBlock];
	unsigned char* buf = new unsigned char[blockSize];
	fetchBlock2(b,buf);
	block = (ext2_inode*)buf;
	return block[i];
}

ext2_inode blockClass::fetchInode(int i) {
	i--;
	int g = (i/file->superBlock.s_inodes_per_group);
	i = (i % file->superBlock.s_inodes_per_group);
	int s = (file->blockGroupDescriptorTable[g].bg_inode_table);
	int b = s + (i/inodesPerBlock);
	i = i % inodesPerBlock;
	ext2_inode* block = new ext2_inode[inodesPerBlock];
	fetchBlock(b, block);
	/*cout << endl << "inode's size at inode: " << block[i].i_size << endl;
    //cout << "indoe blocks: " << block[i].i_blocks << endl;
    cout << "inode links: " << block[i].i_links_count << endl;*/
	return block[i];
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

/*void blockClass::readBitmap(vdiFile* file, char str, int index) {
	
	//file->vdi_lseek((b*blockSize)+partitionStart, SEEK_SET);
	int loc;
	if(str == 'b') {
		loc = file->blockGroupDescriptorTable[index-1].bg_block_bitmap;
	}
	if(str == 'i') {
		loc = file->blockGroupDescriptorTable[index-1].bg_inode_bitmap;
	}
	if(str == 't') {
		loc = file->blockGroupDescriptorTable[index-1].bg_inode_bitmap;
	}
	
	unsigned char *bitmap; 
	bitmap = (unsigned char *)malloc(blockSize);	
	cout << endl << "size of BGDT: " << sizeof(file->blockGroupDescriptorTable[index-1]) << endl;
	cout << endl << "loc: " << loc << endl;
	file->vdi_lseek(partitionStart + ((loc-1)*blockSize), SEEK_SET);
	file->vdi_read(bitmap, blockSize);
	for(int i = 0; i < blockSize; i++) {
		printf("%u ",bitmap[i]);
	}
	
	unsigned char *bitmap2; 
	bitmap2 = new unsigned char[blockSize];
	for(int i = 0; i < blockSize; i++) {
		file->vdi_read(&bitmap, sizeof(bitmap[i]));
		file->vdi_lseek(sizeof(bitmap[i]), SEEK_CUR);
	}
	file->vdi_lseek(partitionStart + ((loc-1)*blockSize), SEEK_SET);
	file->vdi_read(bitmap2, blockSize);
	for(int i = 0; i < blockSize; i++) {
		printf("%u ",bitmap2[i]);
	}
	
	bool same = true;
	for(int i = 0; i < blockSize; i++) {
		if(bitmap[i] != bitmap2[i])
			same = false;
	}
	
	//cout << endl << end << same;
}*/

unsigned char* directory::fetchBlockFromFile(int blkNum, ext2_inode node) {
	unsigned char* test = new unsigned char[bClass->getBlockSize()];
	return test; // needs fixed later
}
	
void directory::openDir(int index) {
	foo = bClass->fetchInode2(index);
	buf = (unsigned char*)malloc(foo.i_size);
	cout << "i_size " << foo.i_size << endl;	//new unsigned char[foo.i_size];
	buf = (unsigned char*)&foo;
	dirCursor = 0;
	/*
	 * cout << endl << "test: " << buf.i_size << endl;
	ext2_inode *pmet = (ext2_inode*)buf;
	cout << endl << "test: " << pmet->i_size << endl;
	unsigned char* buf = (unsigned char*)malloc(temp.i_size);*/
}

int directory::readDir(int& index_number, char* name) {
	struct ext2_dir_entry_2 *pDent = (struct ext2_dir_entry_2*)(buf+dirCursor);	
	{
		cout << endl << "inode " << pDent->inode << endl;
		cout << "rec_len " << pDent->rec_len << endl;
		cout << "name_len " << (int)pDent->name_len << endl;
		cout << "file_type " << (int)pDent->file_type << endl;
		cout << "name: ";
		for(int i = 0; i < pDent->name_len; i++) {
			cout << (char)pDent->name[i] << endl;
		}
		cout << endl << "name 2: " << pDent->name << endl;
		cout << endl;
	}
	if(dirCursor == foo.i_size)
		return 0;
	else {
		if(pDent->inode != 0 && *pDent->name != '.' && *pDent->name != ('.'+'.')) {
			cout << "old dirCursor: " << dirCursor << endl;
			dirCursor += (int)pDent->rec_len;
			cout << "new dirCursor: " << dirCursor << endl;
			index_number = pDent->inode;
			cout << "index just change: " << index_number << endl;
			for(int i = 0; i < pDent -> rec_len; i++) {
				name[i] = pDent->name[i];
			}
			//name += 0;
			
		}
		return 1;
	}
}

void directory::scanDir(int index){ 
	openDir(index);
	//for(int i = 0; i 
}
	
void directory::rewindDir(void) {
	dirCursor = 0;
}

void directory::traverseDirectory() {
	
}
