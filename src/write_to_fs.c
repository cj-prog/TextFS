/*
 * Copyright (C) 2016 - Christian Jürgens <christian.textfs@gmail.com>
 * Copyright (C) 2016 - Dirk Klingenberg <blademountain35@gmail.com>
 * Copyright (C) 2005 - Alejandro Liu Ly <alejandro_liu@hotmail.com>
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


/**************************************************************************************************
 * functions for file system
 **************************************************************************************************/

/*
 * Write Block to file system-image (Current: Zeroblock)
 * @fs					 - file system structure
 * @startAddress - first address
 * @size				 - size
 * */
void writeDataBlock(struct tfs *fs, u8 *startAddress, u16 size)
{
	u8 currentByte = 0;					//0-16 Each Byte per Line and |
	u16 currentLine = 16;				//Every Line contains 15 Byte + |...
	u8 *currentAddress;					//iterates addresses

	dofwrite(fs->fp, "000:\t", 5);

	for (currentAddress = startAddress;
			 currentAddress != (u8 *) startAddress + size; currentAddress++)
	{
		if (currentAddress == (u8 *) startAddress + size - 1)
		{
			while (currentByte < 16)
			{
				if (currentByte == 7)
				{
					fprintf(fs->fp, "%02x  ", (u8) *currentAddress);
					currentByte++;
					currentAddress++;
					size++;
				}
				fprintf(fs->fp, "%02x ", (u8) *currentAddress);
				currentByte++;
				currentAddress++;
				size++;
			}
			writeASCIILine(fs, (u8**) currentAddress);
			dofwrite(fs->fp, "\n\n", 2);
		}
		else
		{
			if (currentByte == 7)
			{
				fprintf(fs->fp, "%02x  ", (u8) *currentAddress);
				currentByte++;
			}
			else if (currentByte == 16)
			{
				writeASCIILine(fs, (u8**) currentAddress);

				fprintf(fs->fp, "\n%03d:\t", currentLine);
				currentLine += 16;
				fprintf(fs->fp, "%02x ", (u8) *currentAddress);
				currentByte = 1;
			}
			else
			{
				fprintf(fs->fp, "%02x ", (u8) *currentAddress);
				currentByte++;
			}
		}
	}
}

/*
 * Write an ASCII-Line to file system-image
 * @fs							- file system structure
 * @currentAddress	- current Address to iterate again
 * */
void writeASCIILine(struct tfs *fs, u8** currentAddress)
{
	u8 *repeatAddress;

	dofwrite(fs->fp, " |", 2);
	for (repeatAddress = (u8 *) currentAddress - 16;
			repeatAddress != (u8 *) currentAddress; repeatAddress++)
		if (*repeatAddress > 32 && *repeatAddress < 127)
		{
			fprintf(fs->fp, "%c", *repeatAddress);
		}
		else
		{
			fprintf(fs->fp, " ");
		}
	fprintf(fs->fp, "|");
}

/*
 * Write bootblock to file system-image
 * @fs							- file system structure
 * @currentAddress	- current Address to iterate again
 * */
void writeBootBlock(struct tfs* fs)
{
	fflush(fs->fp);

	if (fseek(fs->fp, 0, SEEK_SET))
	{
		die("fseek");
	}
	fprintf(fs->fp, "block-id: %lu\n", fs->bb->blockID);
	fprintf(fs->fp, "Fragment-Type: %s\n", fs->bb->fragment_type);
	fprintf(fs->fp, "encoding: %s\n", fs->bb->encoding);
	newline(fs);
}

/*
 * Write superblock to file system-image
 * @fs	- file system structure
 * */
void writeSuperBlock(struct tfs* fs)
{
	fprintf(fs->fp, "block-id: %lu\n", fs->sb->blockID);
	fprintf(fs->fp, "Fragment-Type: %s\n", fs->sb->fragment_type);
	fprintf(fs->fp, "file system-state: %d\n", fs->sb->state);
	fprintf(fs->fp, "zone-bitmap-size_blocks: %d\n", fs->sb->zmap_sizeInBlocks);
	fprintf(fs->fp, "inode-bitmap-size_blocks: %d\n", fs->sb->imap_sizeInBlocks);
	fprintf(fs->fp, "number-of-inodes: %d\n", fs->sb->nInodes);
	fprintf(fs->fp, "number-of-blocks: %d\n", fs->sb->fs_sizeInBlocks);
	fprintf(fs->fp, "first-data-block: %d\n", fs->sb->firstdatazone);

	newline(fs);
}

/*
 * Write freelist to file system-image
 * @fs						- file system structure
 * @sizeInBlocks	- size in blocks
 * */
void writeZoneBMap(struct tfs* fs, int sizeInBlocks)
{
	int free_blocks = 0;
	int i;
	unsigned long blockID;

	get_free_blocks(fs->zone_bmap, sizeInBlocks, &free_blocks);

	for (i = 0, blockID = ZONE_BITMAP_POS; i < fs->sb->zmap_sizeInBlocks;
			 i++, blockID++)
	{
		fprintf(fs->fp, "block-id: %lu\n", blockID);
		fprintf(fs->fp, "Fragment-Type: zone-bitmap\n");
		fprintf(fs->fp, "free-blocks-in-file system: %d\n", free_blocks);
		writeDataBlock(fs, (u8 *) fs->zone_bmap + i * BLOCKSIZE, BLOCKSIZE);
	}
}

/*
 * Write inode bitmap to file system-image
 * @fs	- file system structure
 * */
void writeInodeBMap(struct tfs* fs)
{
	int i;
	unsigned long blockID = ZONE_BITMAP_POS + fs->sb->zmap_sizeInBlocks;

	for (i = 0; i < fs->sb->imap_sizeInBlocks; i++, blockID++)
	{
		fprintf(fs->fp, "block-id: %lu\n", blockID);
		fprintf(fs->fp, "Fragment-Type: inode-bitmap\n");
		writeDataBlock(fs, (u8 *) *(&fs->inode_bmap + i * BLOCKSIZE), BLOCKSIZE);
	}
}

/*
 * Write inodes to file system-image
 * @fs	- file system structure
 * */
void writeInodes(struct tfs* fs)
{
	int i, j;
	unsigned long blockID= ZONE_BITMAP_POS + fs->sb->zmap_sizeInBlocks + fs->sb->imap_sizeInBlocks;

	for (i = 1; i <= INODE_BUFFER_SIZE(fs) / BLOCKSIZE; i++, blockID++)
	{
		fprintf(fs->fp, "block-id: %lu\n", blockID);
		fprintf(fs->fp, "Fragment-Type: inode-%d\n", i);
		fprintf(fs->fp, "file-type: %06d\n", INODE(fs,i)->i_mode);
		fprintf(fs->fp, "links-to-file: %d\n", INODE(fs,i)->i_nlinks);

		for(j = 0; j < 7; j++)
		{
			fprintf(fs->fp, "data-zone[%d]: %d\n", j, INODE(fs,i)->zones[j]);
		}
		fprintf(fs->fp, "indirect-data-zone: %d\n", INODE(fs,i)->indirZone);
		fprintf(fs->fp, "double-indirect-data-zone: %d\n", INODE(fs,i)->doubleIndirZone);
		fprintf(fs->fp, "file-size-in-bytes: %03d\n", INODE(fs,i)->i_size);
		fprintf(fs->fp, "atime: %d\n\n",INODE(fs,i)->i_atime);
	}
}

/*
 * Seek from current position
 * @fs	- file system structure
 * @val	-	seek steps
 * */
void fseekCur(struct tfs *fs, int val)
{
	fflush(fs->fp);

	if (fseek(fs->fp, val, SEEK_CUR))
	{
		die("fseek");
	}
}

/*
 * print newline
 * @fs	- file system structure
 * */
void newline(struct tfs *fs)
{
	dofwrite(fs->fp, "\n", 1);
}

/**************************************************************************************************
 * functions for virtualFS
 **************************************************************************************************/

/*
 * write ASCII line to virtualFS
 * @virtualFS				- file system string
 * @currentAddress	- write address
 * */
void writeVirtualASCIILine(char* virtualFS, u8** currentAddress)
{
	u8 *repeatAddress;

	sprintf(virtualFS, "%s |", virtualFS);

	for (repeatAddress = (u8 *) currentAddress - 16;
			repeatAddress != (u8 *) currentAddress; repeatAddress++)
		if (*repeatAddress > 32 && *repeatAddress < 127)
		{
			sprintf(virtualFS, "%s%c", virtualFS, *repeatAddress);
		}
		else
		{
			sprintf(virtualFS, "%s ", virtualFS);
		}
	sprintf(virtualFS, "%s|", virtualFS);
}

/*
 * write block line to virtualFS
 * @virtualFS				- file system string
 * @zone						- zone
 * @currentAddress	- write address
 * @size						- size
 * */
void writeVirtualDataBlock(char *virtualFS, unsigned long zone,
													 u8 *startAddress, u16 size)
{
	u8 currentByte = 0;					//0-16 Each Byte per Line and |
	u16 currentLine = 16;				//Every Line contains 15 Byte + |...
	u8 *currentAddress;					//iterates addresses
char *storeFS;// = (char *)malloc(strlen(virtualFS), sizeof(char));
	// store old content
	virtualFS = goto_Block(virtualFS, zone);
	storeFS = (char *)malloc(strlen(virtualFS) + 1);
//	char storeFS[strlen(virtualFS)];

	if(virtualFS)
	{
		strcpy(storeFS, virtualFS);
	}
	// add new content
	virtualFS = goto_dataBlk(virtualFS, zone);
	sprintf(virtualFS, "000:\t");

	for (currentAddress = startAddress;
			 currentAddress != (u8 *) startAddress + size; currentAddress++)
	{
		if (currentAddress == (u8 *) startAddress + size - 1)
		{
			while (currentByte < 16)
			{
				if (currentByte == 7)
				{
					sprintf(virtualFS, "%s%02x  ", virtualFS, (u8) *currentAddress);
					currentByte++;
					currentAddress++;
					size++;
				}
				sprintf(virtualFS, "%s%02x ", virtualFS, (u8) *currentAddress);
				currentByte++;
				currentAddress++;
				size++;
			}
			writeVirtualASCIILine(virtualFS, (u8**) currentAddress);
			sprintf(virtualFS, "%s\n\n", virtualFS);
		}
		else
		{
			if (currentByte == 7)
			{
				sprintf(virtualFS, "%s%02x  ", virtualFS, (u8) *currentAddress);
				currentByte++;
			}
			else if (currentByte == 16)
			{
				writeVirtualASCIILine(virtualFS, (u8**) currentAddress);

				sprintf(virtualFS, "%s\n%03d:\t", virtualFS, currentLine);
				currentLine += 16;
				sprintf(virtualFS, "%s%02x ", virtualFS, (u8) *currentAddress);
				currentByte = 1;
			}
			else
			{
				sprintf(virtualFS, "%s%02x ", virtualFS, (u8) *currentAddress);
				currentByte++;
			}
		}
	}

	// restore old content
	if (goto_Block(storeFS, zone + 1))
	{
		sprintf(virtualFS, "%s%s", virtualFS, goto_Block(storeFS, zone + 1));
		free(storeFS);
	}
}

/*
 * Write to a file/inode.  It makes holes along the way...
 * @fs 		- file system structure
 * @fp 		- input file
 * @inode - inode to write to
 * */
void writefile(struct tfs *fs,FILE *fp,int inode)
{
  int j,block_size,block_count = 0;
  u8 block[BLOCKSIZE];
  u32 count = 0;

  do
  {
  	block_size = fread(block,1,BLOCKSIZE,fp);

    for (j=0; j<block_size; j++)
    {
    	if (block[j])
    	{
    		break;
    	}
    }
    if (j != block_size)
    {
    	// This is not a hole, so better write it
      if (block_size < BLOCKSIZE)
      {
      	memset(block+block_size,0,BLOCKSIZE-block_size);
      }
      write_block_to_inode(fs,inode,block_count++,block);
    }
    count += block_size;
  } while (block_size == BLOCKSIZE);

  trunc_inode(fs,inode,count);
}

/*
 * Write data to file/inode.
 * @fs 		- file system structure
 * @blk 	- data to write
 * @cnt 	- bytes to write
 * @inode	- inode to write to
 */
void writedata(struct tfs *fs,u8 *blk,u32 cnt,int inode)
{
  int i,block_count;

  for (block_count=i=0; i < cnt; i+= BLOCKSIZE)
  {
    if (i+BLOCKSIZE < cnt)
    {
    	write_block_to_inode(fs,inode,block_count++,blk+i);
    }
    else
    {
      u8 blk2[BLOCKSIZE];
      memcpy(blk2,blk+i,cnt-i);
      memset(blk2+cnt-i,0,BLOCKSIZE-cnt+i);
      write_block_to_inode(fs,inode,block_count,blk2);
    }
  }
  trunc_inode(fs,inode,cnt);
}

/*
 * Create a symlink
 * @fs 			- file system structure
 * @target	- target link
 * @lnknam 	- link name
 * */
void domklnk(struct tfs *fs,char *target,char *lnknam)
{
  int len = strlen(target);
  int inode = make_node(fs,lnknam,0777|S_IFLNK,0,0,len,NOW,NOW,NOW,NULL);

  writedata(fs,(u8 *)target,len,inode);
}

/*
 * Create a symlink command
 * @fs 		- file system structure
 * @argc 	- from command line
 * @argv 	- from command line
 * */
void cmd_mklnk(struct tfs *fs,int argc,char **argv)
{
  domklnk(fs,argv[3],argv[4]);
}

/*
 * Create a hard link
 * @fs		 - file system structure
 * @target - target link
 * @lnknam - link name
 * */
void domkhlnk(struct tfs *fs,char *target,char *lnknam)
{
  char *dir = lnknam;
  char *lname = strrchr(lnknam,'/');
  int dinode;
  int tinode = find_inode(fs,target);
	char dir_string[64];
	strcpy(dir_string, dir);
	struct tfs_inode *ino =INODE(fs,tinode);

	if (!S_ISREG(ino->i_mode))
	{
		fatalmsg("%s: can only link regular files");
	}
	ino->i_nlinks++;

  if (find_inode(fs,lnknam) != ERROR)
  {
  	fatalmsg("%s: already exists",lnknam);
  }
  if (lname)
  {
		lname++;
		strtok(dir_string, "/");
		dir = dir_string;
  }
  else
  {
    dir = ".";
    lname = lnknam;
  }
  dinode = find_inode(fs,dir);

  if (dinode == ERROR)
  {
  	fatalmsg("%s: not found\n",dir);
  }
  dname_add(fs,dinode,lname,tinode);
}

/*
 * Create a symlink command
 * @fs 	 - file system structure
 * @argc - from command line
 * @argv - from command line
 * */
void cmd_hardlnk(struct tfs *fs,int argc,char **argv)
{
  domkhlnk(fs,argv[3],argv[4]);
}

/*
 * Add files to a image file
 * @fs	 - file system structure
 * @argc - from command line
 * @argv - from command line
 * */
void cmd_add(struct tfs *fs, int argc, char **argv)
{
  FILE *fp;
  struct stat sb;
  int inode, free_blocks;
  float file_blocks;

  if (stat(argv[3],&sb))
  {
  	die("stat(%s)",argv[3]);
  }
  //Check file size
  get_free_blocks(fs->zone_bmap, fs->sb->fs_sizeInBlocks, &free_blocks);
  file_blocks =(float) sb.st_size/(float) 512;
  if(file_blocks > free_blocks)
  {
  	printf("\nThe file %s is too big for filesystem\n", argv[3]);
  	printf("Filesize is %f Bytes\n",file_blocks * 512);
  	printf("Free space of Filesystem is %d Bytes\n\n",free_blocks * 512);
  	exit(0);
  }
  if (!S_ISREG(sb.st_mode))
  {
  	fatalmsg("%s: not a regular file\n",argv[3]);
  }
	if(!strcmp(argv[1], argv[3]))
	{
		printf("\nAdding the file system isn't a valid operation\n\n" );
		exit(0);
	}
  fp = fopen(argv[3],"rb");

  if (!fp)
  {
  	die(argv[3]);
  }
  int length = strlen(argv[4]) - 1;
  char *length_ptr = argv[4];
  length_ptr += length;

  if(*length_ptr != '/')
  {
  	printf("For the root path use [Sourcepath] [/] \n" );
  	printf("For another path use [Sourcepath] [directory/] \n" );
  	exit(0);
  }

	// Get filename
  char *filename = strrchr(argv[3], '/');
	char targetpath[BLOCKSIZE] = { 0 };

	if(!filename)
	{
		filename = argv[3];
	}
	else
	{
		filename++;
	}
	// Put filename to targetpath
	if(argv[4])
	{
		//Check the maximum path length
		if((strlen(argv[4]) + strlen(argv[3])) > BLOCKSIZE)
		{
			printf("The path %s is too long\n",argv[3]);
			exit(0);
		}
		strcpy(targetpath, argv[4]);
	  strcat(targetpath, filename);
	}
	else
	{
		strcpy(targetpath, filename);
	}

  inode = make_node(fs, &targetpath[0], sb.st_mode,	0,0, sb.st_size,sb.st_atime,
																				sb.st_mtime,sb.st_ctime,NULL);

  writefile(fs,fp,inode);
  fclose(fp);
}
