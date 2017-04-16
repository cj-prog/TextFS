/*
 * Copyright (C) 2016 - Christian Jürgens <christian.textfs@gmail.com>
 * Copyright (C) 2016 - Dirk Klingenberg <blademountain35@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * */

#include "spec_tfs.h"
#include "protos.h"
#include <utime.h>

/**************************************************************************************************
 * Functions for virtualFS
 **************************************************************************************************/

/*
 * get key value pair
 * @virtualFS	- file system as string
 * @retVal		-	0 return value as integer
 *							1 return value as string
 * @key				- key to find value
 * */
int getHeaderValue(char* virtualFS, char *retVal, const char* key)
{
	char *ptr;
	char *ptr2;
	int i;

	if ((ptr = strstr(virtualFS, key)))
	{
		ptr2 = strstr(ptr, "\n");
		char value[ptr2 - ptr - strlen(key) + 1];

		for (i = 0; i < ptr2 - ptr - strlen(key); i++)
		{
			value[i] = ptr[i + strlen(key)];
		}
		value[i] = '\0';

		if (retVal)
		{
			strcpy(retVal, value);
		}
		return atoi(value);
	}
	return 0;
}

/*
 * Read only content of file system, and write it to virtualFS
 * @fs	- file system structure
 * @fn	-	file system name
 * */
void readVirtualFS(struct tfs *fs, const char *fn)
{
	struct stat fdstat;

	if (fs->fp && !stat(fn, &fdstat))
	{
		fs->virtualFS = malloc(fdstat.st_size + 1);
	  size_t nread = fread(fs->virtualFS, 1, fdstat.st_size, fs->fp);

	  // Terminate the buffer as a string
	  fs->virtualFS[nread] = '\0';

	  // Truncate the string after the end-of-data:
	  char *endOfData = strstr(fs->virtualFS, "\n ");
	  if (endOfData)
	  {
	  	endOfData[1] = '\0';
	  }
	  else
	  {
	  	fs->sb->state = ERROR;
	  }
	}
	else
	{
		printf("<--> Error: %s konnte nicht geöffnet werden!\n", fn);
	}
}

/*
 * Read Boot block from virtualFS
 * @fs	- file system structure
 * */
void readVirtualBootBlock(struct tfs *fs)
{
	fflush(fs->fp);

	if (fseek(fs->fp, 0, SEEK_SET))
	{
		die("fseek");
	}
	fs->bb = domalloc(BLOCKSIZE, DEFAULTVALTOBESET);
	fs->bb->blockID = getHeaderValue(fs->virtualFS, NULL, "block-id: ");
	getHeaderValue(fs->virtualFS, fs->bb->fragment_type, "Fragment-Type: ");
	getHeaderValue(fs->virtualFS, fs->bb->encoding, "encoding: ");
}

/*
 * Read Super block from virtualFS
 * @fs	- file system structure
 * */
void readVirtualSuperBlock(struct tfs *fs)
{
	fs->sb = domalloc(BLOCKSIZE, DEFAULTVALTOBESET);

	fs->sb->blockID = getHeaderValue(goto_Block(fs->virtualFS, SB_POSITION),
																	 NULL, "block-id: ");

	getHeaderValue( goto_Block(fs->virtualFS, SB_POSITION), fs->sb->fragment_type,
									"Fragment-Type: ");

	fs->sb->state = getHeaderValue( goto_Block(fs->virtualFS, SB_POSITION), NULL,
																	"file system-state: ");

	fs->sb->zmap_sizeInBlocks = getHeaderValue(goto_Block(fs->virtualFS, SB_POSITION),
																						 NULL, "zone-bitmap-size_blocks: ");

	fs->sb->imap_sizeInBlocks = getHeaderValue(goto_Block(fs->virtualFS, SB_POSITION),
																						 NULL, "inode-bitmap-size_blocks: ");

	fs->sb->nInodes = getHeaderValue(goto_Block(fs->virtualFS, SB_POSITION),
																		 NULL,"number-of-inodes: ");

	fs->sb->fs_sizeInBlocks = getHeaderValue(goto_Block(fs->virtualFS, SB_POSITION),
																	 NULL,"number-of-blocks: ");

	fs->sb->firstdatazone = getHeaderValue(goto_Block(fs->virtualFS, SB_POSITION),
																					 NULL, "first-data-block: ");
}

/*
 * Read Zone Bitmap blocks from virtualFS
 * @fs	- file system structure
 * */
void readVirtualZoneBMap(struct tfs *fs)
{
	int i;

	fs->zone_bmap = domalloc((fs)->sb->zmap_sizeInBlocks * BLOCKSIZE, -1);

	for (i = 0; i < fs->sb->zmap_sizeInBlocks; i++)
	{
		readVirtualDataBlock(goto_dataBlk(fs->virtualFS, SB_POSITION + 1 + i),
												(unsigned long) fs->zone_bmap + i * BLOCKSIZE);
	}
}

/*
 * Read Inode Bitmap blocks from virtualFS
 * @fs	- file system structure
 * */
void readVirtualInodeBMap(struct tfs *fs)
{
	int i;

	fs->inode_bmap = domalloc((fs)->sb->imap_sizeInBlocks * BLOCKSIZE, -1);

	for (i = 0; i < fs->sb->imap_sizeInBlocks; i++)
	{
		readVirtualDataBlock( goto_dataBlk(fs->virtualFS,
													SB_POSITION + 1 + ((fs)->sb->zmap_sizeInBlocks) + i),
													(unsigned long) fs->inode_bmap + i * BLOCKSIZE);
	}
}

/*
 * Read Inode blocks from virtualFS
 * @fs	- file system structure
 * */
void readVirtualInodes(struct tfs* fs)
{
	int i, j, j_to_c;

	fs->inode = domalloc(INODE_BUFFER_SIZE(fs), -1);

	for (i = 1; i <= fs->sb->nInodes; i++)
	{
		INODE(fs,i)->i_mode =
								getHeaderValue(goto_Block( fs->virtualFS,
																   				 SB_POSITION
																					 + ((fs)->sb->zmap_sizeInBlocks)
																					 + (fs->sb->imap_sizeInBlocks)
																					 + i),
																				 	 NULL, "file-type: ");

		INODE(fs,i)->i_nlinks =
								getHeaderValue(goto_Block( fs->virtualFS,
																     			 SB_POSITION
																					 + ((fs)->sb->zmap_sizeInBlocks)
																					 + (fs->sb->imap_sizeInBlocks)
																					 + i),
																					 NULL, "links-to-file: ");
		for(j = 0; j < 7; j++)
		{
			char data_zone[14] = "data-zone[j]: ";
			j_to_c = j + 0x30;
			data_zone[10] = j_to_c;

			INODE(fs,i)->zones[j] =
								getHeaderValue(goto_Block( fs->virtualFS,
																					 SB_POSITION
																					 + ((fs)->sb->zmap_sizeInBlocks)
																					 + (fs->sb->imap_sizeInBlocks)
																					 + i),
															 	 	 	 	 	 	 NULL, data_zone);
		}
		INODE(fs,i)->indirZone =
								getHeaderValue(goto_Block( fs->virtualFS,
																					 SB_POSITION
																					 + ((fs)->sb->zmap_sizeInBlocks)
																					 + (fs->sb->imap_sizeInBlocks)
																					 + i),
															 	 	 	 	 	 	 NULL, "indirect-data-zone: ");

		INODE(fs,i)->doubleIndirZone =
								getHeaderValue(goto_Block( fs->virtualFS,
																					 SB_POSITION
																					 + ((fs)->sb->zmap_sizeInBlocks)
																					 + (fs->sb->imap_sizeInBlocks)
																					 + i),
															 	 	 	 	 	 	 NULL, "double-indirect-data-zone: ");

		INODE(fs,i)->i_size =
								getHeaderValue(goto_Block( fs->virtualFS,
																					 SB_POSITION
																					 + ((fs)->sb->zmap_sizeInBlocks)
																					 + (fs->sb->imap_sizeInBlocks)
																					 + i),
															 	 	 	 	 	 	 NULL, "file-size-in-bytes: ");

		INODE(fs,i)->i_atime =
								getHeaderValue(goto_Block( fs->virtualFS,
																					 SB_POSITION
																					 + ((fs)->sb->zmap_sizeInBlocks)
																					 + (fs->sb->imap_sizeInBlocks)
																					 + i),
															 	 	 	 	 	 	 NULL, "atime: ");
	}
}

/*
 * Seek file to the specified block
 * @virtualFS	- file system as string
 * @blk 			- block to go to
 * @return		- ptr to block or NULL
 * */
char *goto_dataBlk(char *virtualFS, unsigned long blk)
{
	char blockID[2 * MAX_SZ_IN_BLOCKS_LENGTH] =	{ 0 };
	char *ptr;

	// Seek "blockID: xx", then begin of datablock and return

	sprintf(blockID, "block-id: %lu", blk);
	if ((ptr = strstr(virtualFS, blockID)))
	{
		return (strstr(ptr, "000:"));
	}
	return NULL;
}

/*
 * Go to needed block
 * @virtualFS	- file system as string
 * @blk 			- block to go to
 * @return		- ptr to block or NULL
 * */
char *goto_Block(char *virtualFS, unsigned long blk)
{

	char blockID[2 * MAX_SZ_IN_BLOCKS_LENGTH] =	{ 0 };
	char *ptr;

	// Seek "blockID: xx" and return
	sprintf(blockID, "block-id: %lu", blk);

	if ((ptr = strstr(virtualFS, blockID)))
	{
		return (strstr(ptr, "block-id:"));
	}
	return NULL;
}

/*
 * read data block from virtualFS
 * @BlockPtr	- ptr to block to read from
 * @address 	- store block to address
 * */
void readVirtualDataBlock(char* BlockPtr, unsigned long address)
{
	char dataBlock[3];
	int hex, i;
	int k;
	u16 m = 0;

	for (i = 0; i < BLOCKSIZE / 16; i++)
	{
		for (k = 5; k < 54; k = k + 3, m++)
		{
			if (k == 29)
			{
				k++;
			}
			sprintf(dataBlock, "%c%c", BlockPtr[k], BlockPtr[k + 1]);
			hex = (unsigned char) strtol(dataBlock, NULL, 16);
			memset((unsigned char*) address + m, hex, 1);
		}
		BlockPtr = BlockPtr + DATA_LINE_WIDTH - 1;
	}

}

/*
 * print formatted block
 * @fs			- file system structure
 * @blockID	- block to be printed
 * */
void printBlock(struct tfs *fs, unsigned long blockID)
{
	unsigned char ausg_c[16];
	int j, k;
	int i = 0;

	if (fseek(fs->fp, blockID * BLOCKSIZE, SEEK_SET))
	{
		die("fseek");
	}
	printf("\n%03d:   ", i);

	for (; i < BLOCKSIZE / 16; i++)
	{
		if (fread(ausg_c, sizeof(char), 16, fs->fp))
		{
			for (k = 0; k < 16; k++)
			{
				printf("%02x ", ausg_c[k]);
				if (k == 7)
				{
					printf(" ");
				}
			}
			printf("|");

			for (j = 0; j < 16; j++)
			{
				if ((int) ausg_c[j] > 32 && (int) ausg_c[j] < 127)
				{
					printf("%c", ausg_c[j]);
				}
				else
				{
					printf(" ");
				}
			}
			printf("|\n");

			if ((i + 1) * 16 != BLOCKSIZE)
			{
				printf("%03d:   ", (i + 1) * 16);
			}
		}
		else
		{
			die("fread");
		}
	}
}

/*
 * print formatted bitmaps
 * @fs			- file system structure
 * */
void print_bitmaps(struct tfs *fs)
{
	int j;
	unsigned char *ptr;

	printf("\ninode bitmap\n");
	printf("pointer start address: %p\n", fs->inode_bmap);
	ptr = fs->inode_bmap;

	for (j = 0; j < ((fs)->sb->imap_sizeInBlocks * BLOCKSIZE); ++j)
	{
		printf("%02x ", *ptr);
		ptr++;
	}
	printf("\n\nzone bitmap\n");
	printf("pointer start address: %p\n", fs->zone_bmap);
	ptr = fs->zone_bmap;

	for (j = 0; j < ((fs)->sb->zmap_sizeInBlocks * BLOCKSIZE); ++j)
	{
		printf("%02x ", *ptr);
		ptr++;
	}
	printf("\n\ninode\n");
	printf("pointer start address: %p\n", fs->inode);

	ptr = (unsigned char *) fs->inode;

	for (j = 0; j < INODE_BUFFER_SIZE(fs); ++j)
	{
		printf("%02x ", *ptr);
		ptr++;
	}
	printf("\n");
}

/**************************************************************************************************
 *
 **************************************************************************************************/

/*
 * Read a file from file system
 * @fs 			- file system structure
 * @fp 			- uutput file
 * @path 		- file to read
 * @type 		- read type: S_IFREG or S_IFLNK
 * @ispipe	- if true we do not use fseek to skip holes
 */
int readfile(struct tfs *fs,FILE *fp,const char *path,int type,int ispipe)
{
  int inode = find_inode(fs,path);
  int i,bsz,j, blk_size;
  u8 blk[BLOCKSIZE];
  int fdirsize;

  if (inode == -1)
  {
  	fatalmsg("%s: not found",path);
 	}
	struct tfs_inode *ino = INODE(fs,inode);

	if (type == S_IFREG && !S_ISREG(ino->i_mode))
	{
		fatalmsg("%s: is not a regular file",path);
	}
	else if (type == S_IFLNK && !S_ISLNK(ino->i_mode))
	{
		fatalmsg("%s: is not a symbolic link",path);
	}
	fdirsize = ino->i_size;

  for (i = 0; i < fdirsize; i += BLOCKSIZE)
  {
    bsz = read_inoblk(fs,inode,i / BLOCKSIZE,blk);

    if (bsz)
    {
    	// Search for \0
    	for(blk_size = 0; blk[blk_size] != '\0'; blk_size++);

    	if(blk_size != 0)
    	{
    		bsz = blk_size - 2;
    	}
      dofwrite(fp,blk,bsz);
    }
    else
    {
      bsz = fdirsize - i > BLOCKSIZE ? BLOCKSIZE : fdirsize % BLOCKSIZE;

      if (ispipe)
      {
      	for (j=0;j<bsz;j++) putc(0,fp);
      }
      else
      {
        fseek(fp,bsz,SEEK_CUR);
      }
    }
  }
  return inode;
}

/*
 * Similar to UNIX cat command
 * @fs 	 - file system structure
 * @argc - from command line
 * @argv - from command line
 * */
void cmd_cat(struct tfs *fs,int argc,char **argv)
{
  int i;

  for (i = 3; i < argc; i++)
  {
  	printf("Content of %s\n\n", argv[i]);
    readfile(fs,stdout,argv[i],S_IFREG,1);
  }
}

/*
 * Extract image files to a normal file
 * @fs 		- file system structure
 * @argc	- from command line
 * @argv 	- from command line
 * */
void cmd_extract(struct tfs *fs,int argc,char **argv)
{
  FILE *fp;
  struct utimbuf tb;
  int mode,inode;

  fp = fopen(argv[4],"wb");

  if (!fp)
  {
  	die(argv[4]);
  }
  inode = readfile(fs,fp,argv[3],S_IFREG,0);

  // We want to copy also the modes ...
	struct tfs_inode *ino = INODE(fs,inode);
	tb.modtime = tb.actime = ino->i_atime;
	mode = ino->i_mode & 07777;

  fclose(fp);

  chmod(argv[4],mode);
  utime(argv[4],&tb);
}

/*
 * Read contents of a symlink
 * @fs 	 - file system structure
 * @argc - from command line
 * @argv - from command line
 * */
void cmd_readlink(struct tfs *fs,int argc,char **argv)
{
  if (argc == 1)
  {
    fatalmsg("Usage: %s [links ...]\n",argv[0]);
  }
  else if (argc == 2)
  {
    readfile(fs,stdout,argv[1],S_IFLNK,1);
    putc('\n',stdout);
  }
  else
  {
    int i;

    for (i = 3;i < argc; i++)
    {
      printf("%s: ",argv[i]);
      readfile(fs,stdout,argv[i],S_IFLNK,1);
      putc('\n',stdout);
    }
  }
}
