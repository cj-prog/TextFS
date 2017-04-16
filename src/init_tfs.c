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


#include "protos.h"
#include "spec_tfs.h"


/*
 * Initializes a new file system
 * File will be created or truncated.
 * @fn 							- file name for new file system
 * @sizeInBlocks 		- size of file system in blocks
 * @numberOfInodes	- number of numberOfInodes to allocate (0 for auto)
 * @return 					- pointer to a tfs structure
 * */
struct tfs *new_tfs(const char *fn, unsigned long sizeInBlocks,
										int numberOfInodes)
{
	struct tfs *fs = domalloc(sizeof(struct tfs), DEFAULTVALTOBESET);
	unsigned long rootblkp;
	char rootblk[BLOCKSIZE];

	initBootBlock(fs, sizeInBlocks);
	initSuperBlock(fs, sizeInBlocks);

	manage_inodes(fs, &numberOfInodes, &sizeInBlocks);

	initBitmaps(fs, sizeInBlocks, numberOfInodes);
	initInodeTables(fs, &rootblkp);

	createFile(fs, fn);

	writeBootBlock(fs);
	writeSuperBlock(fs);
	writeZoneBMap(fs, sizeInBlocks);
	writeInodeBMap(fs);
	writeInodes(fs);

	initRootBlock(fs, (char *) &rootblk, rootblkp);
	writeDataBlock(fs, (u8 *) (&rootblk[0]), BLOCKSIZE);

	fclose(fs->fp);
	free_memory(fs);

	return fs;
}

/*
 * Open a file system
 * @fn 		 - file name for new file system
 * @chk 	 - stop if file system is not clean
 * @return - pointer to a minix_fs_dat structure
 * */
struct tfs *open_fs(const char *fn)
{
	struct tfs *fs = domalloc(sizeof(struct tfs), DEFAULTVALTOBESET);

	fs->fp = fopen(fn, "r+b");

	if (!fs->fp)
	{
		die(fn);
	}
	readVirtualFS(fs, fn);
	readVirtualBootBlock(fs);
	readVirtualSuperBlock(fs);
	readVirtualZoneBMap(fs);
	readVirtualInodeBMap(fs);
	readVirtualInodes(fs);

	// Sanity check
	if (TFS_VALID != fs->sb->state)
	{
		fprintf(stderr, "Warning: %s in an unknown state\n", fn);
	}

	return fs;
}

/*
 * Closes file system
 * @fs 		 - pointer to file system structure
 * @return - NULL
 * */
struct tfs *close_fs(struct tfs *fs)
{
	int sizeInBlocks = fs->sb->fs_sizeInBlocks;

	writeBootBlock(fs);
	writeSuperBlock(fs);
	writeZoneBMap(fs, sizeInBlocks);
	writeInodeBMap(fs);
	writeInodes(fs);

	fputs(goto_Block(fs->virtualFS, fs->sb->firstdatazone), fs->fp);
	fclose(fs->fp);

	free_memory(fs);

	return 0;
}
