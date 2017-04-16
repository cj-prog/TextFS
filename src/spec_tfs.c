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

/*
 * Initialize bootblock
 * @fs						- file system structure
 * @sizeInBlocks	-	file system size in blocks
 * */
void initBootBlock(struct tfs *fs, unsigned long sizeInBlocks)
{
	int defaultValToBeSet = DEFAULTVALTOBESET;
	fs->bb = domalloc(BLOCKSIZE, defaultValToBeSet);

	fs->bb->blockID = ZB_POSITION;
	strcpy(fs->bb->fragment_type, "partition-description");
	strcpy(fs->bb->encoding, "iso-8859-1");
}

/*
 * Initialize superblock
 * @fs						- file system structure
 * @sizeInBlocks	-	file system size in blocks
 * */
void initSuperBlock(struct tfs *fs, unsigned long sizeInBlocks)
{
	int defaultValToBeSet = 0;
	fs->sb = domalloc(BLOCKSIZE, defaultValToBeSet);

	fs->sb->blockID = SB_POSITION;
	strcpy(fs->sb->fragment_type, "superblock");
	fs->sb->fs_sizeInBlocks = sizeInBlocks;
	fs->sb->state = TFS_VALID;
}

/*
 * Initialize bitmaps
 * @fs							- file system structure
 * @sizeInBlocks		-	file system size in blocks
 * @numberOfInodes	- number of inodes
 * */
void initBitmaps(struct tfs *fs, unsigned long sizeInBlocks, int numberOfInodes)
{
	int i;

	(fs)->sb->imap_sizeInBlocks = UPPER(numberOfInodes, BITS_PER_BLOCK);
	(fs)->sb->zmap_sizeInBlocks =	UPPER(sizeInBlocks, BITS_PER_BLOCK);

	fs->zone_bmap = domalloc((fs)->sb->zmap_sizeInBlocks * BLOCKSIZE, 0xff);
	fs->inode_bmap = domalloc((fs)->sb->imap_sizeInBlocks * BLOCKSIZE, 0xff);
	(fs)->sb->firstdatazone = NORM_FIRSTZONE(fs);

	for (i = (fs)->sb->firstdatazone; i < sizeInBlocks; i++)
	{
		unmark_zone(fs, i);
	}
	for (i = TFS_ROOT_INO; i < (fs)->sb->nInodes; i++)
	{
		unmark_inode(fs, i + 1);
	}
}

/*
 * Initialize inode tables
 * @fs			- file system structure
 * @rootblk	-	rootblock
 * */
void initInodeTables(struct tfs *fs, unsigned long *rootblk)
{
	int defaultValToBeSet = DEFAULTVALTOBESET;

	fs->inode = domalloc(INODE_BUFFER_SIZE(fs), defaultValToBeSet);

	set_inode(fs, TFS_ROOT_INO, S_IFDIR | 0755, 2, 2 * DIRSIZE(fs), NOW, NOW, NOW,
						0, defaultValToBeSet);

	*rootblk = get_free_block(fs);
	INODE(fs,TFS_ROOT_INO)->zones[0] = *rootblk;

	mark_zone(fs, *rootblk);
}

/*
 * Initialize root block
 * @fs				- file system structure
 * @rootblk		-	rootblock
 * @rootblkp	- rootblock position
 * */
void initRootBlock(struct tfs *fs, char *rootblk, unsigned long rootblkp)
{
		memset(rootblk, 0, BLOCKSIZE);
		*((short *) rootblk) = TFS_ROOT_INO;
		strcpy(rootblk + 2, ".");
		*((short *) (rootblk + DIRSIZE(fs))) = TFS_ROOT_INO;
		strcpy(rootblk + 2 + DIRSIZE(fs), "..");

		fprintf(fs->fp, "block-id: %lu\n", rootblkp);
		fprintf(fs->fp, "Fragment-Type: index-block\n");
}
