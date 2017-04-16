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
 * Compare filenames in a directory structure
 * @lname - name to compare
 * @dname - directory name to compare
 * @len		- length of directory structure
 * */
int cmp_name(const char *lname, const char *dname, int nlen)
{
	int i;

	for (i = 0; i < nlen; i++)
	{
		if (lname[i] == '/' || lname[i] == 0)
		{
			if (!dname[i])
			{
				return 0;
			}
			return ERROR;
		}
		if (lname[i] != dname[i])
			return 1;
	}
	return 0;
}

/*
 * Find name in a directory
 * Update *blkp and *offp if name is found
 * @fs 			- file system structure
 * @inode 	- inode for directory to search
 * @lname 	- filename (nul or "/" terminated)
 * @blkp 		- file block where name was found
 * @offp 		- offset pointer where name was found
 * @return	- inode for found name, -1 on error.
 * */
int ilookup_name( struct tfs *fs, int inode, const char *lname,
									u32 *blkp, int *offp)
{
	int i, bsz, j;
	u8 blk[BLOCKSIZE];
	int fdirsize = (INODE(fs, inode))->i_size;
	int dentsz = DIRSIZE(fs);

	for (i = 0; i < fdirsize; i += BLOCKSIZE)
	{
		bsz = read_inoblk(fs, inode, i / BLOCKSIZE, blk);
		for (j = 0; j < bsz; j += dentsz)
		{
			u16 fino = *((u16 *) (blk + j));
			if (!fino)
			{
				continue;
			}
			if (!cmp_name(lname, (char*)blk + j + 2, dentsz - 2))
			{
				if (blkp)
				{
					*blkp = i / BLOCKSIZE;
				}
				if (offp)
				{
					*offp = j;
				}
				return fino;
			}
		}
	}
	return ERROR;
}

/*
 * Add file name to a directory.
 * @fs 		- file system structure
 * @dir 	- path to directory
 * @name 	- name to add
 * @inode	- inode number
 */
void dname_add(struct tfs *fs, int dinode, const char *name, int inode)
{
	u8 blk[BLOCKSIZE] = {0};
	int dentsz = DIRSIZE(fs);
	int dfsize = INODE(fs,dinode)->i_size;
	int i, j = 0, nblk = 0, bsz;

	for (i = 0; i < dfsize; i += BLOCKSIZE)
	{
		bsz = read_inoblk(fs, dinode, nblk = i / BLOCKSIZE, blk);

		for (j = 0; j < bsz; j += dentsz)
		{
			u16 fino = *((u16 *) (blk + j));
			if (!fino)
			{
				goto SKIP_ALL;
			}
		}
	}
	SKIP_ALL:
	if (i >= dfsize)
	{
		//Need to extend directory file
		INODE(fs,dinode)->i_size += dentsz;

		if (j == BLOCKSIZE)
		{
			nblk++;
			j = 0;
			memset(blk, 0, BLOCKSIZE);
		}
	}
	else
	{
		memset(blk + j, 0, dentsz);
	}
	// Create directory entry
	*((u16 *) (blk + j)) = inode;
	strncpy((char*)blk + j + 2, name, dentsz - 2);

	// Update directory
	write_block_to_inode(fs, dinode, nblk, blk);
}

/*
 * Remove file name from a directory.
 * @fs 			- file system structure
 * @dir 		- path to directory
 * @name 		- name to add
 * @return	- 0 in success, -1 in error.
 */
void dname_rem(struct tfs *fs, int dinode, const char *name)
{
	u8 blk[BLOCKSIZE];
	int dentsz = DIRSIZE(fs);
	u32 nblk;
	int off;
	int i;

	i = ilookup_name(fs, dinode, name, &nblk, &off);

	if (i == ERROR)
	{
		return;
	}

	i = (INODE(fs,dinode)->i_size) - dentsz;

	if (i == (nblk * BLOCKSIZE + off))
	{
		// Need to shorten directory file
		INODE(fs,dinode)->i_size = i;

		if (!(i % BLOCKSIZE))
		{
			// We should free-up the last block...
			free_inoblk(fs, dinode, (i / BLOCKSIZE) + 1);
		}
	}
	else
	{
		read_inoblk(fs, dinode, nblk, blk);
		memset(blk + off, 0, dentsz);
		write_block_to_inode(fs, dinode, nblk, blk);
	}
}

/*
 * Create a new inode/directory
 * @fs 				- file system structure
 * @fpath 		- filename path
 * @mode 			- mode
 * @uid 			- user id
 * @gid 			- group id
 * @size 			- file size
 * @atime 		- access time
 * @mtime 		- modification time
 * @ctime			- change time
 * @dinode_p	- pointer to the directory that contains this node
 */
int make_node(struct tfs *fs, char *fpath, int mode,
							int uid, int gid, u32 size,
							u32 atime, u32 mtime, u32 ctime, int *dinode_p)
{
	char *dir = fpath;

	// Copy path to string
	char dir_string[64];
	strcpy(dir_string, dir);

	// Get filename
	char *fname = strrchr(fpath, '/');
	int dinode, ninode;

	// If file exist in directory, then break
	if (find_inode(fs, fpath) != ERROR)
	{
		fatalmsg("%s: already exists", fpath);
	}
	// Get directory without filename
	if (fname)
	{
		char *get_slash;
		fname++;
		get_slash = strrchr(dir_string,'/');
		*get_slash = '\0';
		dir = dir_string;
	}
	else
	{
		dir = ".";
		fname = fpath;
	}
	dinode = find_inode(fs, dir);

	if (dinode == ERROR)
	{
		fatalmsg("%s: not found\n", dir);
	}

	ninode = get_free_inode(fs);
//	if(ninode > fs->sb->nInodes)
//	{
//		printf("\nNo free Inode found\n");
//		exit(0);
//	}
	mark_inode(fs, ninode);

	// Initialize inode
	set_inode(fs, ninode, mode, 1, size, atime, mtime, ctime, 1, 0);
	dname_add(fs,dinode,fname,ninode);

	if (dinode_p)
	{
		 *dinode_p = dinode;
	}

	return ninode;
}
