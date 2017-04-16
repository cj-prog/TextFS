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
#include "bitops.h"

/*
 * Build header for block number
 * @fs 				- file system structure
 * @inode 		- fs -> inode
 * @zone 			- block number
 * @option 		- INDIRECT_BLOCK,
 * 						-	DOUBLE_INDIRECT_BLOCK,
 * 						-	INDEX_OR_DATA_BLOCK,
 * 						-	INDEX_BLOCK
 * @inode_cnt	- inode number
 * */
void build_header(struct tfs *fs, struct tfs_inode *inode, unsigned long zone,
									int option, int inode_cnt)
{
	sprintf(fs->virtualFS, "%sblock-id: %lu\n", fs->virtualFS,
					(long unsigned int) zone);

	if (option == INDIRECT_BLOCK)
	{
		sprintf(fs->virtualFS,
						"%sFragment-Type: indirect-data-block-from-inode-%d \n",
						fs->virtualFS, inode_cnt);
	}
	else if (option == DOUBLE_INDIRECT_BLOCK)
	{
		sprintf(fs->virtualFS,
						"%sFragment-Type: double-indirect-data-block-from-inode-%d \n",
						fs->virtualFS, inode_cnt);
	}
	else if (option == INDEX_OR_DATA_BLOCK)
	{
		if (inode->i_mode == 16877)
		{
			sprintf(fs->virtualFS, "%sFragment-Type: index-block-from-inode-%d\n",
							fs->virtualFS, inode_cnt);
		}
		else if (inode->i_mode == 33204 || inode->i_mode == 33188)
		{
			sprintf(fs->virtualFS, "%sFragment-Type: data-block-from-inode-%d\n",
							fs->virtualFS, inode_cnt);
		}
	}
	else if (option == INDEX_BLOCK)
	{
		sprintf(fs->virtualFS, "%sFragment-Type: index-block-from-inode-%d\n",
						fs->virtualFS, inode_cnt);
	}
	sprintf(fs->virtualFS, "%s000:", fs->virtualFS);
}

/*
 * Manage inodes
 * Set default, round up, limit and check upper boundedness
 * @fs 							- file system structure
 * @numberOfInodes	- number of inodes
 * @sizeInBlocks 		- size in blocks
 * */
void manage_inodes( struct tfs *fs, int *numberOfInodes,
										unsigned long *sizeInBlocks)
{
	if (!*numberOfInodes)
	{
		// Default number inodes to 1/3 blocks
		*numberOfInodes = *sizeInBlocks / 3;
	}

	// Round up inode count
	*numberOfInodes = ((*numberOfInodes + INODES_PER_BLOCK - 1)
										& ~(INODES_PER_BLOCK - 1));

	if (*numberOfInodes > 65535)
	{
		*numberOfInodes = 65535;
	}

	(fs)->sb->nInodes = *numberOfInodes;

	if (INODE_BLOCKS(fs) > *sizeInBlocks * 9 / 10 + 5)
	{
		fatalmsg("Too many inodes requested");
	}
}

/*
 * Calculate the block number from zone content
 * @fs 			- file system structure
 * @inode		- inode structure (v1)
 * @zoneID	- zone number in current inode
 * @return	- blockID
 * */
int get_blockID_from_inode( struct tfs *fs, struct tfs_inode *inode,
														int zoneID)
{

	// Direct block
	if (zoneID < NR_OF_DIREKT_ZONES)
	{
		return inode->zones[zoneID];
	}

	// Indirect block
	zoneID -= NR_OF_DIREKT_ZONES;

	if (zoneID < ADRESSES_PER_BLOCK)
	{
		u16 indir_zone[ADRESSES_PER_BLOCK];

		if (inode->indirZone == 0)
		{
			return 0;
		}

		// Get indirect block with block numbers of data blocks
		readVirtualDataBlock( goto_dataBlk(fs->virtualFS, inode->indirZone),
													(unsigned long) indir_zone);

		// Return block number
		return indir_zone[zoneID];
	}

	// Double indirect block.
	zoneID -= ADRESSES_PER_BLOCK;

	if (zoneID < ADRESSES_PER_BLOCK * ADRESSES_PER_BLOCK)
	{
		u16 indir_zone[ADRESSES_PER_BLOCK];
		if (inode->doubleIndirZone == 0)
		{
			return 0;
		}

		// Get indirect zone with block numbers of double indirect blocks
		readVirtualDataBlock(goto_dataBlk(fs->virtualFS, inode->doubleIndirZone),
													(unsigned long) indir_zone);

		if (indir_zone[zoneID / ADRESSES_PER_BLOCK] == 0)
		{
			return 0;
		}

		// Get double indirect block with block numbers of data blocks
		unsigned long search_block = indir_zone[zoneID / ADRESSES_PER_BLOCK];

		readVirtualDataBlock( goto_dataBlk(fs->virtualFS, search_block),
													(unsigned long) indir_zone);

		// Return data block
		return indir_zone[zoneID % ADRESSES_PER_BLOCK];
	}

	die("file bigger than maximum size");

	return ERROR;
}

/*
 * Sets the block for a file's inode to point to specific zone (v1)
 * @fs 						- file system structure
 * @inode 				- inode to update (v1)
 * @zoneID	- File block to update
 * @blockID 			- blockID to point to
 * */
void write_blockID_to_inode(struct tfs *fs, struct tfs_inode *inode,
														int zoneID, int blockID, int w_inode)
{
	// Direct block
	if (zoneID < NR_OF_DIREKT_ZONES)
	{
		if (inode->zones[zoneID] && inode->zones[zoneID] != blockID)
		{
			unmark_zone(fs, inode->zones[zoneID]);
		}

		inode->zones[zoneID] = blockID;
		return;
	}

	//Indirect block
	zoneID -= NR_OF_DIREKT_ZONES;

	if (zoneID < ADRESSES_PER_BLOCK)
	{
		u16 indir_zone[ADRESSES_PER_BLOCK];

		if (inode->indirZone == 0)
		{
			inode->indirZone = get_free_block(fs);
			mark_zone(fs, inode->indirZone);
			memset(indir_zone, 0, sizeof indir_zone);
		}
		else
		{
			// Read Data-Block from indirect_data_zone
			readVirtualDataBlock( goto_dataBlk(fs->virtualFS, inode->indirZone),
														(unsigned long) indir_zone);

			if (indir_zone[zoneID]	&& indir_zone[zoneID] != blockID)
			{
				unmark_zone(fs, indir_zone[zoneID]);
			}
		}

		indir_zone[zoneID] = blockID;

		// Create a new indirect_data_zone or set new entry
		if (!goto_dataBlk(fs->virtualFS, inode->indirZone))
		{
			build_header(fs, inode, inode->indirZone, INDEX_BLOCK, w_inode);
		}

		writeVirtualDataBlock(fs->virtualFS, inode->indirZone,
													(u8 *) (indir_zone), BLOCKSIZE);
		return;
	}

	// Double indirect block
	zoneID -= ADRESSES_PER_BLOCK;

	if (zoneID < ADRESSES_PER_BLOCK * ADRESSES_PER_BLOCK)
	{
		u16 indir_zone[ADRESSES_PER_BLOCK];
		u16 double_indirect_blockID, double_indirect_block;

		if (inode->doubleIndirZone == 0)
		{
			inode->doubleIndirZone = get_free_block(fs);
			mark_zone(fs, inode->doubleIndirZone);
			memset(indir_zone, 0, sizeof indir_zone);
		}
		else
		{
			//Read indirect block from double_indirect_zone and write it to indir_zone
			readVirtualDataBlock( goto_dataBlk(fs->virtualFS, inode->doubleIndirZone),
														(unsigned long) indir_zone);
		}

		double_indirect_blockID = zoneID / ADRESSES_PER_BLOCK;
		zoneID %= ADRESSES_PER_BLOCK;

		if (indir_zone[double_indirect_blockID] == 0)
		{
			double_indirect_block = indir_zone[double_indirect_blockID]
														=	get_free_block(fs);

			mark_zone(fs, indir_zone[double_indirect_blockID]);

			// Create a new double_indirect_data_zone(block-number)
			if (!goto_dataBlk(fs->virtualFS, inode->doubleIndirZone))
			{
				build_header(fs, inode, inode->doubleIndirZone, INDEX_BLOCK, w_inode);
			}

			writeVirtualDataBlock(fs->virtualFS, inode->doubleIndirZone,
														(u8 *) (indir_zone), BLOCKSIZE);

			memset(indir_zone, 0, sizeof indir_zone);
		}
		else
		{
			// Read from double_indirect_block and write it to indir_zone
			double_indirect_block = indir_zone[double_indirect_blockID];

			readVirtualDataBlock( goto_dataBlk(fs->virtualFS, double_indirect_block),
														(unsigned long) indir_zone);

			if (indir_zone[zoneID]	&& indir_zone[zoneID] != blockID)
			{
				unmark_zone(fs, indir_zone[zoneID]);
			}
		}

		indir_zone[zoneID] = blockID;

		// Create a new data-block for indir_zone at double_indirect_block or set new entry
		if (!goto_dataBlk(fs->virtualFS, double_indirect_block))
		{
			build_header(fs, inode, double_indirect_block, INDEX_BLOCK, w_inode);
		}

		writeVirtualDataBlock(fs->virtualFS, double_indirect_block,
													(u8 *) (indir_zone), BLOCKSIZE);
		return;
	}
	die("file bigger than maximum size");
}

/*
 * Frees the block for a file's inode (v1)
 * @fs 						- file system structure
 * @inode 				- inode to update (v1)
 * @zoneID	- File block to update
 * */
void delete_blockID_from_inode( struct tfs *fs, struct tfs_inode *inode,
																int zoneID, int w_inode)
{
	int i;

	// Direct block
	if (zoneID < NR_OF_DIREKT_ZONES)
	{
		if (inode->zones[zoneID])
		{
			unmark_zone(fs, inode->zones[zoneID]);
		}
		inode->zones[zoneID] = 0;
		return;
	}

	// Indirect block
	zoneID -= NR_OF_DIREKT_ZONES;
	if (zoneID < ADRESSES_PER_BLOCK)
	{
		u16 indir_zone[ADRESSES_PER_BLOCK];
		if (!inode->indirZone)
		{
			return;
		}

		// Get indir_zone
		readVirtualDataBlock( goto_dataBlk(fs->virtualFS, inode->indirZone),
													(unsigned long) indir_zone);

		//Delete blockID from indirect_block
		if (indir_zone[zoneID])
		{
			unmark_zone(fs, indir_zone[zoneID]);
		}
		indir_zone[zoneID] = 0;

		//Write new indirect_block (without deleted blockID) to virtualFS
		for (i = 0; i < ADRESSES_PER_BLOCK; i++)
		{
			if (indir_zone[i])
			{
				if (!goto_dataBlk(fs->virtualFS, inode->indirZone))
				{
					build_header(fs, inode, inode->indirZone, INDIRECT_BLOCK, w_inode);
				}
				writeVirtualDataBlock(fs->virtualFS, inode->indirZone,
															(u8 *) (indir_zone), BLOCKSIZE);
				return;
			}
		}

		//If indirect_block is empty, delete pointer to it
		unmark_zone(fs, inode->indirZone);
		inode->indirZone = 0;
		return;
	}
	// Double indirect block.
	zoneID -= ADRESSES_PER_BLOCK;

	if (zoneID < ADRESSES_PER_BLOCK * ADRESSES_PER_BLOCK)
	{
		u16 indir_zone[ADRESSES_PER_BLOCK];
		u16 double_indir_zone[ADRESSES_PER_BLOCK];
		u16 double_indirect_blockID;

		if (!inode->doubleIndirZone)
		{
			return;
		}
		// Get indir_zone
		readVirtualDataBlock( goto_dataBlk(fs->virtualFS, inode->doubleIndirZone),
													(unsigned long) indir_zone);

		double_indirect_blockID = zoneID / ADRESSES_PER_BLOCK;
		zoneID %= ADRESSES_PER_BLOCK;

		if (!indir_zone[double_indirect_blockID])
		{
			return;
		}

		// Get double_indir_zone
		readVirtualDataBlock(goto_dataBlk(fs->virtualFS,
												 indir_zone[double_indirect_blockID]),
												 (unsigned long) double_indir_zone);

		// Delete blockID from double_indirect_block
		if (double_indir_zone[zoneID])
		{
			unmark_zone(fs, double_indir_zone[zoneID]);
		}
		double_indir_zone[zoneID] = 0;

		// Write new double_indirect_block to virtualFS
		for (i = 0; i < ADRESSES_PER_BLOCK; i++)
		{
			if (double_indir_zone[zoneID])
			{
				if (!goto_dataBlk(fs->virtualFS, indir_zone[double_indirect_blockID]))
				{
					build_header( fs, inode, indir_zone[double_indirect_blockID],
					              DOUBLE_INDIRECT_BLOCK, w_inode);
				}
				writeVirtualDataBlock(fs->virtualFS,
															indir_zone[double_indirect_blockID],
															(u8 *) (indir_zone), BLOCKSIZE);
				return;
			}
		}

		// If double_indirect_block is empty, delete it from indirect_block
		unmark_zone(fs, indir_zone[double_indirect_blockID]);
		indir_zone[double_indirect_blockID] = 0;

		// Write new indirect_block to virtualFS
		for (i = 0; i < ADRESSES_PER_BLOCK; i++)
		{
			if (indir_zone[zoneID])
			{
				if (!goto_dataBlk(fs->virtualFS, inode->doubleIndirZone))
				{
					build_header(fs, inode, inode->doubleIndirZone, DOUBLE_INDIRECT_BLOCK, w_inode);
				}
				writeVirtualDataBlock(fs->virtualFS, inode->doubleIndirZone,
															(u8 *) (indir_zone), BLOCKSIZE);
				return;
			}
		}
		// If indirect_block is empty, delete pointer to it
		unmark_zone(fs, inode->doubleIndirZone);
		inode->doubleIndirZone = 0;
		return;
	}
	die("file bigger than maximum size");
}

/*
 * Read an inode block.
 * Checks if we are actually trying to read past the end-of-file.
 * @fs 			- file system structure
 * @inode		- inode to read from
 * @blk 		- file block to read
 * @buf 		- buffer pointer (must be BLOCKSIZE)
 * @return	-	number of bytes copied in to buf (typically BLOCKSIZE) unless
 *						last block or if we are reading a hole, which will return 0.
 */
int read_inoblk(struct tfs *fs, int r_inode, u32 blk, u8 *buf)
{
	int blockID;
	int bsize = BLOCKSIZE;
	struct tfs_inode *inode = INODE(fs, r_inode);

	if (blk * BLOCKSIZE > inode->i_size)
	{
		return 0;
	}

	blockID = get_blockID_from_inode(fs, inode, blk);

	if (!blockID)
	{
		// This is a hole!
		return 0;
	}
  if (inode->i_size / BLOCKSIZE == blk)
  	bsize = inode->i_size % BLOCKSIZE;

	readVirtualDataBlock( goto_dataBlk(fs->virtualFS, blockID),
												(unsigned long) buf);
	if (bsize < BLOCKSIZE)
		memset(buf+bsize,0,BLOCKSIZE-bsize);

	return bsize;
}

/*
 * Write an inode block.
 * It will allocate blocks from zone_bmap as needed.
 * It will *not* extend the filesize counter in the inode.
 * @fs 						- file system structure
 * @inode 				- inode to write to
 * @zoneID	- file block to write
 * @buf 					- buffer pointer (must be BLOCKSIZE)
 * */
void write_block_to_inode(struct tfs *fs, int w_inode, u32 zoneID,
													u8 *buf)
{
	unsigned long blockID;
	struct tfs_inode *inode = INODE(fs, w_inode);

	blockID = get_blockID_from_inode(fs, inode, zoneID);

	if (!blockID)
	{
		// Allocate block...
		blockID = get_free_block(fs);
		mark_zone(fs, blockID);

		if (!goto_dataBlk(fs->virtualFS, blockID))
		{
			build_header(fs, inode, blockID, INDEX_OR_DATA_BLOCK, w_inode);
		}

		writeVirtualDataBlock(fs->virtualFS, blockID, (u8 *) (buf), BLOCKSIZE);
		write_blockID_to_inode(fs, inode, zoneID, blockID, w_inode);
	}
	else
	{
		if (!goto_dataBlk(fs->virtualFS, blockID))
		{
			build_header(fs, inode, blockID, INDEX_OR_DATA_BLOCK, w_inode);
		}

		writeVirtualDataBlock(fs->virtualFS, blockID, (u8 *) (buf), BLOCKSIZE);
	}
}

/*
 * Free an inode block.
 * @fs 		- file system structure
 * @inode	- inode to free
 * @blk		- file block free
 * */
void free_inoblk(struct tfs *fs, int inode, u32 blk)
{
	delete_blockID_from_inode(fs, INODE(fs, inode), blk, inode);
}

/*
 * Trim file to size
 * @fs 		- File system structure
 * @inode	- inode to trim
 * @sz 		- new file size
 * */
void trunc_inode(struct tfs *fs, int t_inode, u32 sz)
{
	struct tfs_inode *inode = INODE(fs, t_inode);

	if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
	{
		return;
	}
	if (sz < inode->i_size)
	{
		int blk = UPPER(sz, BLOCKSIZE);
		int eblk = UPPER(inode->i_size, BLOCKSIZE);

		while (blk < eblk)
		{
			delete_blockID_from_inode(fs, inode, blk++, t_inode);
		}
	}
	inode->i_size = sz;

}

/*
 * Find an inode given the specified path
 * @fs 			- file system structure
 * @path 		- path to find
 * @return	- inode number or -1 on error
 * */
int find_inode(struct tfs *fs, const char *path)
{
	int inode = TFS_ROOT_INO;

	if (!strcmp(path, "/") || path[0] == 0)
	{
		path = ".";
	}
	else if (path[0] == '/')
		++path;

	while (path)
	{
		inode = ilookup_name(fs, inode, path, NULL, NULL);

		if (inode == ERROR)
		{
			return ERROR;
		}
		path = strchr(path, '/');

		if (path)
		{
			path++;
		}
	}
	return inode;
}

/*
 * Change inode configuration
 * @fs 			- file system structure
 * @inode 	- inode to update
 * @mode 		- mode
 * @nlinks	- number of links
 * @uid 		- user id
 * @gid 		- group id
 * @size 		- file size or rdev type
 * @atime 	- access time
 * @mtime 	- modification time
 * @ctime 	- change time
 * @clr 		- if true, we will clear the inode first...
 */
void set_inode( struct tfs *fs, int inode, int mode, int nlinks, u32 size,
								u32 atime, u32 mtime, u32 ctime, int clr,
								int defaultValToBeSet)
{
	time_t now = time(NULL);
	struct tfs_inode *ino = INODE(fs, inode);

	if (clr)
	{
		memset(ino, defaultValToBeSet, sizeof(struct tfs_inode));
	}
	ino->i_mode = mode;
	ino->i_nlinks = nlinks;

	if (S_ISCHR(ino->i_mode) || S_ISBLK(ino->i_mode))
	{
		ino->i_size = 0;
		ino->zones[0] = size;
	}
	else
	{
		ino->i_size = size;
	}
	ino->i_atime = atime == NOW ? now : atime;
	ino->i_mtime = mtime == NOW ? now : mtime;
	ino->i_ctime = ctime == NOW ? now : ctime;

}

/*
 * Clear inode entry
 * @fs 		- file system structure
 * @inode	- inode to update
 * */
void clr_inode(struct tfs *fs, int inode)
{
	struct tfs_inode *ino = INODE(fs, inode);
	memset(ino, 0, sizeof(struct tfs_inode));
	unmark_inode(fs, inode);
}
