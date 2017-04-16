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

/*
 * includes, constants, structures and prototypes of the TextFS
 * */

#ifndef SPEC_TFS_H_
#define SPEC_TFS_H_

#include <stdio.h>
#include "bitops.h"
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define VERSION "1.0, 2016"
#define ZB_POSITION 0
#define SB_POSITION	1
#define ZONE_BITMAP_POS 2
#define TFS_ROOT_INO 1

//unsigned long, bits:32,  0 .. 4294967295
#define DEFAULT_INODES 10
#define MINIMUM_INODES 1
#define MAXIMUM_INODES 1000
#define DEFAULT_BLOCKS 1000
#define MINIMUM_BLOCKS 30
#define MAXIMUM_BLOCKS 65536
#define ZEROBLOCK 1
#define SUPERBLOCK 1
#define ZONE_BITMAP 2
#define INODE_BITMAP 1

#define MAX_SZ_IN_BLOCKS_LENGTH 11
#define NR_OF_DIREKT_ZONES 7
#define INDIRECT_BLOCK 1
#define DOUBLE_INDIRECT_BLOCK 2
#define INDEX_OR_DATA_BLOCK 3
#define INDEX_BLOCK 4
#define KEY_SIZE 32
#define VALUE_SIZE 32
#define DATA_LINE_WIDTH 75
#define FINISH 1
#define ENDLINE 1
#define DATABEGIN	2
#define ERROR -1
#define DEFAULTVALTOBESET 0
#define BLOCKSIZE 512
#define BLOCKSIZE_BRUTTO 2450
#define DATA_BLOCKSIZE 512
#define HEADER_BLOCKSIZE 88
#define NOT_FOUND 0
#define FOUND 1
#define BITS_PER_BLOCK	(BLOCKSIZE << 3) // BLOCKSIZE * 8
#define INODES_PER_BLOCK 1
#define TFS_VALID 0x0001

#define ADRESSES_PER_BLOCK	(BLOCKSIZE/sizeof(u16))

// round off, only whole inodes..
#define UPPER(size,bitsPerBlock) ( ( size + bitsPerBlock - 1 ) / bitsPerBlock )
#define INODE_BLOCKS(fs) UPPER((fs)->sb->nInodes, INODES_PER_BLOCK)
#define INODE_BUFFER_SIZE(fs) (INODE_BLOCKS(fs) * BLOCKSIZE)
#define NORM_FIRSTZONE(fs) (2+ ((fs)->sb->imap_sizeInBlocks) + ((fs)->sb->zmap_sizeInBlocks) + INODE_BLOCKS(fs))
#define DIRSIZE(fs) 32
#define INODE(fs,inodep) ((fs)->inode + ((inodep)-1))
#define mark_inode(fs,blockID) (setbit((char*)(fs)->inode_bmap,(blockID - 1)))
#define unmark_inode(fs,blockID) (clrbit((char*)(fs)->inode_bmap,(blockID - 1)))
#define mark_zone(fs,datazone) (setbit((char*)(fs)->zone_bmap,(datazone)-((fs)->sb->firstdatazone)))
#define mark_zone_new(fs,datazone) (setbit((char*)(fs)->zone_bmap,(datazone)))
#define unmark_zone(fs,x) (clrbit((char*)(fs)->zone_bmap,(x)-((fs)->sb->firstdatazone)))
#define unmark_zone_new(fs,datazone) (clrbit((char*)(fs)->zone_bmap,(datazone)))
#define get_free_inode(fs) get_free_bit((fs)->inode_bmap,(fs)->sb->imap_sizeInBlocks) + 1
#define get_free_block(fs) ( get_free_bit( (fs)->zone_bmap, (fs)->sb->zmap_sizeInBlocks )	+ ((fs)->sb->firstdatazone) )
#define NOW ((u32)-1)

/*
 * Bootblock configuration
 * */
struct tfs_bootblock
{
	unsigned long blockID;
	char fragment_type[VALUE_SIZE];
	char encoding[VALUE_SIZE];
};

/*
 * Superblock configuration
 * */
struct tfs_superblock
{
	unsigned long blockID;
	char fragment_type[VALUE_SIZE];
	u16 nInodes; 								 	// Number of inodes
	u16 nZones; 									// Device size in blocks (v1)
	u16 imap_sizeInBlocks; 				// Inode map size in blocks
	u16 zmap_sizeInBlocks; 				// Zone map size in blocks
	u16 firstdatazone; 						// Where data blocks begin
	u16 log_zone_size; 						// unused
	u32 maxFileSize; 							// Max file size supported in bytes
	u16 version; 						// fs version
	u16 state; 										// file system state
	u32 fs_sizeInBlocks; 					// device size in blocks (v2)
	u32 unused[4];								// unused
};

/*
 * The new minix inode has all the time entries, as well as
 * long block numbers and a third indirect block (7+1+1+1
 * instead of 7+1+1). Also, some previously 8-bit values are
 * now 16-bit. The inode is now 64 bytes instead of 32.
 */
struct tfs_inode
{
	u16 i_mode; 										// File type and permissions for file
	u16 i_nlinks; 									// Hard link count
	u16 i_uid;
	u16 i_gid;
	u32 i_size;											// File size in bytes
	u32 i_atime;
	u32 i_mtime;
	u32 i_ctime;
	u32 zones[7];
	u32 indirZone;
	u32 doubleIndirZone;
	u32 _unused;
};

/*
 * Text file system configuration
 * */
struct tfs
{
	FILE *fp;
	char *virtualFS;
	struct tfs_bootblock *bb;
	struct tfs_superblock *sb;
	struct tfs_inode *inode;
	u8 *inode_bmap;
	u8 *zone_bmap;								// Free Blocks in Bitmap

};

/*
 * Global options
 */
#ifndef EXTERN
#define EXTERN(a,b) extern a
#endif
EXTERN(int opt_squash, 0);
EXTERN(int opt_fsbad_fatal, 0);

#endif /* SPEC_TFS_H_ */
