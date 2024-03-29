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
#include "include/types.h"

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

	int fd = open(argv[1], O_RDONLY);	
	if(fd == -1) {	
		
		fprintf(stderr, "image not found.\n");
		exit(1);

	}
	
	int rc;
	struct stat sbuf;
	rc = fstat(fd, &sbuf);
	assert(rc == 0);
	
	void *img_ptr;
	img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0); 

	
	assert(img_ptr != MAP_FAILED);


	struct superblock *sb;
 	sb = (struct superblock *) (img_ptr + BSIZE);	

	struct dinode *dip = (struct dinode *) (img_ptr + 2*BSIZE);
	 dip = (struct dinode *) (img_ptr + 2*BSIZE);
	 for(int i = 0; i < sb->ninodes ; i++) {
		
		printf("loop: %d location: %d\n", i, dip->addrs[0]);
		if(dip->type == T_DIR) {
			printf("-----------------\n");
			for(int k = 0 ; k < NDIRECT + 1; k++ ) {
				printf("directory inode contents: %d\n", dip->addrs[k]);
			} 

			struct dirent * d = (struct dirent *)(img_ptr + dip->addrs[0]*BSIZE);
			for(int j = 0 ; j < BSIZE/sizeof(struct dirent) ; j++) {
		
				printf("directory name: %s directory inum: %d directory datablock: %d \n", d->name, d->inum, dip->addrs[0]);
				d++;
			} 
		
		} 
		dip++;


	
	} 


}
