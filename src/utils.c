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


#include "protos.h"
#include "spec_tfs.h"

/*
 * Find a free bit in map
 * @bmap 		- bitmap to scan
 * @bsize 	- bitmap size in blocks
 * @return	- the bit number of the found block
 */
unsigned long get_free_bit(u8 *bmap, int bsize)
{
	int i;
	int j;

	for (i = 0; i < bsize * BLOCKSIZE; i++)
	{
		if (bmap[i] != 0xff)
		{
			for (j = 0; j < 8; j++)
			{
				if ((bmap[i] & (1 << j)) == 0)
				{
					return (i << 3) + j;
				}
			}
			die("Internal error!");
		}
	}
	fatalmsg("No free slots in bitmap found");
	return ERROR;
}

/*
 * Allocate memory (w/error handling) and memory clearing.
 * @size 		- number of bytes to allocate
 * @elm 		- Initialize memory to this value
 * 						-1 for no initialization
 * @return	- newly allocated memory.
 * */
void *domalloc(unsigned long size, int defaultValToBeSet)
{
	void *ptr = malloc(size);

	if (!ptr)
	{
		die("malloc");
	}
	if (defaultValToBeSet >= 0)
	{
		memset(ptr, defaultValToBeSet, size);
	}
	return ptr;
}

/*
 * Print an error message and die.
 * @s - string format
 * */
void fatalmsg(const char *s, ...)
{
	va_list p;
	va_start(p, s);
	vfprintf(stderr, s, p);
	va_end(p);
	putc('\n', stderr);
	exit(ERROR);
}

/*
 * Like fatalmsg but also show the errno message.
 * @s - string format
 * */
void die(const char *s, ...)
{
	va_list p;
	va_start(p, s);
	vfprintf(stderr, s, p);
	va_end(p);
	putc(':', stderr);
	putc(' ', stderr);
	perror(NULL);
	putc('\n', stderr);
	exit(errno);
}

/*
 * Create file with specified size
 * - Open file
 * - check for opening failure
 * - set file pointer on right position
 * - put 0 on the last position
 * @fs	-	file system structure
 * @fn	- file system name
 */
void createFile(struct tfs *fs, const char *fn)
{
	unsigned long i;

	fs->fp = fopen(fn, "w+b");

	if (!fs->fp)
	{
		die(fn);
	}
	for (i = 0; i < fs->sb->fs_sizeInBlocks * BLOCKSIZE_BRUTTO - 1; i++)
	{
		putc(' ', fs->fp);
	}
	if (fseek(fs->fp, fs->sb->fs_sizeInBlocks * BLOCKSIZE_BRUTTO - 1, SEEK_SET))
	{
		die("fseek");
	}
	fflush(fs->fp);
}

/*
 * Write cnt bytes to file.
 * @fp 			- file system file.
 * @buff 		- data to write
 * @cnt 		- bytes to write
 * @return	- buff
 * */
void *dofwrite(FILE *fp, void *buff, int cnt)
{
	if (cnt != fwrite(buff, 1, cnt, fp))
	{
		die("fwrite");
	}
	fflush(fp);
	return buff;
}

/*
 * Read cnt bytes from file
 * @fp 			- file system file.
 * @buff 		- buffer (BLOCKSIZE) big.
 * @cnt 		- bytes to read
 * @return	- buff
 * */
void *dofread(FILE *fp, void *buff, int cnt)
{
	if (cnt != fread(buff, 1, cnt, fp))
	{
		die("fread");
	}
	return buff;
}

/*
 * free allocated memory
 * @fs	- file system structure
 * */
void free_memory(struct tfs* fs)
{
	free(fs->bb);
	fs->bb = NULL;
	free(fs->sb);
	fs->sb = NULL;
	free(fs->inode_bmap);
	fs->inode_bmap = NULL;
	free(fs->zone_bmap);
	fs->zone_bmap = NULL;
	free(fs->inode);
	fs->inode = NULL;
	free(fs->virtualFS);
	fs->virtualFS = NULL;
	free(fs);
	fs = NULL;
}

/*
 * get free blocks
 * @bmap				- bitmap
 * @bsize				- bitmap size
 * @free_blocks	- free blocks
 * */
void get_free_blocks(u8 *bmap, int bsize, int *free_blocks)
{
	int i;
	int j;

	*free_blocks = 0;

	for (i = 0; i < bsize/8 ; i++)
	{
		if (bmap[i] != 0xff)
		{
			for (j = 0; j < 8; j++)
			{
				if ((bmap[i] & (1 << j)) == 0)
				{
					*free_blocks += 1;
				}
			}
		}
	}
}
