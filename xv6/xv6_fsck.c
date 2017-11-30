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

	// Test 2 - check if the addresses are valid by checking if all the data
	// block addresses are in between the location of the data bitmap and the sizeof the image pointer

	dip = (struct dinode *) (img_ptr + 2*BSIZE);

	// run through the inodes
	for(i = 0; i < sb->ninodes; i++) { 
		// check if the inodes are valid
		if(dip->type > 0 && dip->type < 4) { 
			//check if the indirect address is valid 
			

			if((dip->addrs[12] < (IBLOCK(sb->ninodes)+1) && (dip->addrs[12] != 0))|| dip->addrs[12] > sbuf.st_size/BSIZE) { 
				fprintf(stderr, "ERROR: bad indirect address in inode\n");
				printf("type of inode: %d address: %d \n", dip->type, dip->addrs[12]);		
			}

			// check if each direct address is valid
			for(int j = 0 ; j < NDIRECT ; j++)  {
					if(dip->addrs[j] < (IBLOCK(sb->ninodes) &&  (dip->addrs[12] != 0)) || dip->addrs[j] > sbuf.st_size/BSIZE) { 
                                		fprintf(stderr, "ERROR: bad direct address in inode\n");
		                                printf("type of inode: %d address: %d\n", dip->type, dip->addrs[j]);

	                      		}
 
 	  		}
			
			


		}
		dip++;
	}

	// Test 3 	

	dip = (struct dinode *) (img_ptr + 2*BSIZE);
	dip++;
	struct dirent *d;

			
			
		fflush(stdout);
		// check if the . directory exists
		d = (struct dirent *)( img_ptr + dip->addrs[0]*512);
		if(strcmp(d->name, ".") != 0) { 
			fprintf(stderr, "ERROR: root directory does not exist\n");
		}

		if(d->inum != 1) { 
                        fprintf(stderr, "ERROR: root directory does not exist\n");
		}
		
		// check if the .. directory exists
		d++;
		if(strcmp(d->name, "..") != 0) { 
			fprintf(stderr, "ERROR: root directory does not exist\n");
		}
		
		// check if the inum of the parent directory is also is 1	
		if(d->inum != 1) { 
			fprintf(stderr, "ERROR: root directory does not exist\n");
		}
 			


	// Test 4


	// go through each inode 
	dip = (struct dinode *) (img_ptr + 2*BSIZE);
	
	for(i = 0 ; i < sb->ninodes; i++)   { 

		// check if the type of the inode is a directory
		if(dip->type == T_DIR) { 
				
			// check if the . exists
			d = (struct dirent *) (img_ptr + dip->addrs[0]*512);
        	        if(strcmp(d->name, ".") != 0) {
	                       fprintf(stderr, "ERROR: directory not properly formatted\n");
                	}

			// check if the inum number matches the inode number - indicating that the . entry points to the directory itself
			if(d->inum != i) { 
                               fprintf(stderr, "ERROR: directory not properly formatted\n");
			}
		
			// check if the .. entry exists
			d++;
			if(strcmp(d->name, "..") != 0) { 
                               fprintf(stderr, "ERROR: directory not properly formatted\n");			
			}
			
		}
		dip++;
	
	}


	// Test 5
		

	dip = (struct dinode *) (img_ptr + 2*BSIZE);
	
	// run through the inodes
	for(i = 0 ; i < sb->ninodes ; i++) { 

		// ensure the inode type is valid 
		if(dip->type > 0 && dip->type < 4) { 

			// run through the addresses 
			for(int j = 0; j < NDIRECT + 1; j++) {

				// ensure the address is not 0 and valid
				if(dip->addrs[j] != 0) { 
		
										


				}
			

			} 
		
		
		}



	}	

}

