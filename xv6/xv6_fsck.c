#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

#define ROOTNO 1
#define BSIZE 512

#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Special device

#define NDIRECT 12
#define DIRSIZ 14


// File system super block
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
};

struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};


struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i)     ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14


#define NDIRECT 12


int main(int argc, char * argv[]) { 

	int fd = open("fs.img", O_RDONLY);	
	assert(fd > -1);

	int rc;
	struct stat sbuf;
	rc = fstat(fd, &sbuf);
	assert(rc == 0);
	
	void *img_ptr;
	img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0); 
	assert(img_ptr != MAP_FAILED);


	struct superblock *sb;
 	sb = (struct superblock *) (img_ptr + BSIZE);	

	// Test 1 : Check if the inodes are either unallocated or one of the valid types
	int i;
	struct dinode *dip = (struct dinode *) (img_ptr + 2*BSIZE);	
 	for(i = 0; i < sb->ninodes; i++) { 
		//printf("type: %d\n", dip->type);
		if(dip->type < 0 || dip->type > 3) { 
			fprintf(stderr, "ERROR: bad inode");	
		}
		dip++;
	}

	// Test 2 -
	/*

	Notes: 28 blocks later lies the bitmap for the data blocks	

	*/	
	
	// get the first inode position	
	dip = (struct dinode *) (img_ptr + 2*BSIZE);
	int bblock;
		
	// run through the inode table
	for(i = 0 ; i < sb->ninodes ; i++)  {
		
		// check if the inodes are in use
		if(dip->type > 0  && dip->type < 4) { 
			
			// loop through the direct addresses 
			for(int j = 0 ; j < NDIRECT ; j++) {  

				//printf("inode: %d data block: %d bblock: %ld\n", i, dip->addrs[j], BBLOCK(dip->addrs[j], sb->ninodes));		
				bblock = BBLOCK(dip->addrs[j], sb->ninodes);	
				
			
				
			

		


			}
	
		} 
		dip++;
	}

 			
	

	

	// figure out the bitmap

	// do other stuff (rest of p5);

	
	return  0;
	

}

