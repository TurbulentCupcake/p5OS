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
	uint byte_number, bit_number;
	uchar actual_byte, value;
	// run through the inodes
	for(i = 0 ; i < sb->ninodes ; i++) { 

		// ensure the inode type is valid 
		if(dip->type > 0 && dip->type < 4) { 

			// run through the addresses 
			for(int j = 0; j < NDIRECT + 1; j++) {
				// ensure the address is not 0 and valid
				if(dip->addrs[j] != 0 && (dip->addrs[j] > (IBLOCK(sb->ninodes)+1) && dip->addrs[j] < sbuf.st_size/BSIZE)) { 
			
				
					// figure which byte the address value belongs to
					byte_number = dip->addrs[j]/8; 				
					// get the bit offset		
					bit_number = dip->addrs[j]%8 ;
					// get the actual byte position from the beginning 
					actual_byte = (uchar)*((uchar*)img_ptr + (IBLOCK(sb->ninodes)+1)*BSIZE + byte_number);	
					// onbtain the correct value by bit shifting
					//value = (actual_byte) & ( (7-bit_number) <<  1);
					value = (actual_byte >> bit_number) & 1;				
					//printf("value: %d index: %d block addrs: %d\n", value, i, dip->addrs[j]);
					if(value != 1) { 
						//printf("bit number: %d\n value: %d\n", bit_number, value);
						fprintf(stderr ,"ERROR: address used by inode but marked free in bitmap\n");
					}

				}
			

			} 
		
		
		}



		dip++;
	}	

	// Test 6
		
        dip = (struct dinode *) (img_ptr + 2*BSIZE);
	uint data_block;
	uint validDirectInodes[sb->ninodes*(NDIRECT+1)];
	uint validIndirectInodes[100000];
	uint posDirect, posIndirect;
	posDirect= 0;
	posIndirect = 0;
	// go through each inode and store all valid addresses
	for(i = 0 ; i < sb->ninodes; i++) { 
		if(dip->type > 0 && dip->type < 4) { 	
		for(int j = 0; j < NDIRECT+1; j++) { 
	
			 if(dip->addrs[j] != 0 && (dip->addrs[j] > (IBLOCK(sb->ninodes)+1) && dip->addrs[j] < sbuf.st_size/BSIZE)) { 
				validDirectInodes[posDirect] = dip->addrs[j];
				posDirect++;
				//printf("pos: %d\n", pos);
			}

		}
		// if we have an indrect address, then check through all the addresses inside that indirect block
		  if(dip->addrs[12] != 0 && (dip->addrs[12] > (IBLOCK(sb->ninodes)+1) && dip->addrs[12] < sbuf.st_size/BSIZE)) {
			

				uint * dblock,*  indirect_addr;
				dblock = (uint*)(img_ptr + dip->addrs[12]*BSIZE);				

				for(uint  l = 0; l < BSIZE/sizeof(uint); l++)  {
						indirect_addr = (uint*)(dblock+l);	
						
					//	printf("Indirect address : %d block addr: %d \n", *indirect_addr, dip->addrs[12]);
		     			        if(*indirect_addr != 0 && (*indirect_addr > (IBLOCK(sb->ninodes)+1) && *indirect_addr < sbuf.st_size/BSIZE)) {
							//	printf("Indirect address : %d\n", *indirect_addr);
								validIndirectInodes[posIndirect] = (uint)*indirect_addr;
								posIndirect++;	
                                                 }

				}
                 }
		}

		dip++;
	} 	
	/*
	printf("List of all direct addresses: \n");
	for( i = 0 ; i < posDirect; i++) { 
		printf("%d ", validDirectInodes[i]);
		if(i % 8 == 0) printf("\n");
	}
		
	printf("\n List of all indirect addresses: \n");
	for(i = 0; i < posIndirect; i++) { 
		printf("%d ", validIndirectInodes[i]);
		if(i%8 == 0) printf("\n"); 
	}
	
	printf("------------------------\n");
	*/
	// for every byte in the final 
	for(i = 0 ; i < BSIZE ; i++) { 

		// get the actual byte 
                actual_byte = (uchar)*((uchar*)img_ptr + (IBLOCK(sb->ninodes)+1)*BSIZE + i);
		for(int j = 0; j < 8 ; j++) { 
			// get the actual bit

			uint foundFlag = 0;
			if(((actual_byte >> j) & (1)))  {
					
						
					// obtain the correct block number
					data_block = i*8 + j;				
					
					if(data_block <= (IBLOCK(sb->ninodes)+1)) continue;	
					
					// check if the block number is present within our array of valid nodes
					for(int k = 0 ; k < posDirect; k++) { 
	 					if(validDirectInodes[k] == data_block) { 
							foundFlag = 1;				
							break;
						}
					}
					
					// if it wasnt found in the direct addresses, then try finding it in the indirect addresses
					if(foundFlag != 1) { 
						for(int k=0; k < posIndirect; k++) { 
							if(validIndirectInodes[k] == data_block) { 
								foundFlag = 1;	
								break;
							}	

						}
					}
					if(foundFlag!=1) { 
									
							printf("bblock value: %ld block number: %d \n", BBLOCK(data_block, sb->ninodes), data_block);
							printf("block no: %d\n", data_block);	
							printf("Error\n");
					}

					
			}
		}
	}
	



	// Test 7	

		
	/* using the two arrays containing the direct and indirect address that we had obtained from the previous tests
		we can use them to find out if there are any direct or indirect address repeats			
				
	*/
	int cmpfunc(const void *a, const void *b)
	{
 		   return (*(int *) a - *(int *) b);
	}
	//Sort the array of direct addresses
	qsort(validDirectInodes, posDirect, sizeof(uint), cmpfunc); 
	
	/*
	for(i=0; i < posDirect; i++) { 
		printf("%d ", validDirectInodes[i]);
	}
	*/
	// check if adjacent elements are the same
	for(i=1; i < posDirect; i++) { 
		
		if(validDirectInodes[i-1] == validDirectInodes[i]){  
			fprintf(stderr, "ERROR: direct address used more than once\n");
		}

	}

	// Test 8

	qsort(validIndirectInodes, posIndirect, sizeof(uint), cmpfunc);

	/*	
	for(i=0; i < posIndirect;i++) { 
		printf("%d ", validIndirectInodes[i]);
	}
	*/
	
	for(i=1; i < posIndirect; i++) { 
		if(validIndirectInodes[i-1] == validIndirectInodes[i]) { 
			fprintf(stderr, "ERROR: indirect address used more than once\n");
		}
	}


	// Test 9

	/* 
	This test is weird, so let me see if i can reason this correctly.
	We know that each file/directory/special device has an inode associated with it.
	This means that each directory has files that are associated with it and each of these
	files would need to be refereced to a certain inode in the set of inodeos
	*/

	uint inums[10000];
	uint inumsCount = 0;
	// let us run through the set of inodes to find those inodes which are directories
		
        dip = (struct dinode *) (img_ptr + 2*BSIZE);
	for(i=0; i < sb->ninodes;i++) { 

		if(dip->type == T_DIR) { 
			//printf("--------------------------------------\n");						
			//printf("directory inode: %d \n", i);
			d = (struct dirent *)( img_ptr + dip->addrs[0]*512);
			for(int j = 0; j < BSIZE/sizeof(struct dirent) ; j++) { 
				//printf("loop: %d\n", j);
				if(d->inum <= sb->ninodes) { 
						//printf("directory inode value: %d directory name: %s\n", d->inum,d->name);
						inums[inumsCount] = d->inum;		
						inumsCount++;

				}
				d++;
			}
		}
		dip++;
	}
	
	// iterate though all the collected inums to find out if it exists

	int referenced;
	for(i=0; i < inumsCount; i++)  { 
			
		referenced = 0;
		for(int j=0 ; j < 200; j++) { 
			if(inums[i] == j){
				 referenced = 1; 
				break;	
			}
		}
		if(!referenced) { 
			fprintf(stderr , "ERROR: inode marked use but not found in a directory\n"); 
		}
		

	}

	// Test 10

	/* let me try to understand what is exactly happening here.
	So we should basically go through each directory entry
}



