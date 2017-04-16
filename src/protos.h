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


#ifndef PROTOS_H_
#define PROTOS_H_

//general
#include "spec_tfs.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include "sfml.h"

//main.c
int main(int argc, char **argv);
void do_cmd(int argc, char **argv);
void generalUsage(const char* name);

//gen_tfs.c
void cmd_mkfs(int argc, char **argv);
void get_size_parameters(int argc, char **argv, unsigned long *nblks_p, int *inodes_p);

//utils.c
void *domalloc(unsigned long size, int defaultValToBeSet);
void die(const char *s, ...);
void *dofread(FILE *fp, void *buff, int cnt);
void *dofwrite(FILE *fp, void *buff, int cnt);
void fatalmsg(const char *s, ...);
void createFile(struct tfs *fs, const char *fn);
void free_memory(struct tfs* fs);
void get_free_blocks(u8 *bmap, int bsize, int *free_blocks);

//inode.c
void delete_blockID_from_inode(struct tfs *fs, struct tfs_inode *inode, int blk, int w_inode);
int get_blockID_from_inode(struct tfs *fs, struct tfs_inode *inode, int blk);
void set_inode( struct tfs *fs, int inode, int mode, int nlinks, u32 size,
								u32 atime, u32 mtime, u32 ctime, int clr,	int defaultValToBeSet);
void manage_inodes(struct tfs *fs, int *numberOfInodes,	unsigned long *sizeInBlocks);
void write_block_to_inode(struct tfs *fs, int inode, u32 blk, u8 *buf);
void trunc_inode(struct tfs *fs, int t_inode, u32 sz);
int find_inode(struct tfs *fs, const char *path);
int read_inoblk(struct tfs *fs, int r_inode, u32 blk, u8 *buf);
void free_inoblk(struct tfs *fs, int inode, u32 blk);
void clr_inode(struct tfs *fs, int inode);

//iname.c
int ilookup_name(struct tfs *fs, int inode, const char *lname, u32 *blkp, int *offp);
int make_node(struct tfs *fs, char *fpath, int mode,int uid, int gid, u32 size,
							u32 atime, u32 mtime, u32 ctime, int *dinode_p);
void dname_add(struct tfs *fs, int dinode, const char *name, int inode);
void dname_rem(struct tfs *fs, int dinode, const char *name);

//init_tfs.c
unsigned long get_free_bit(u8 *bmap, int bsize);
struct tfs *open_fs(const char *fn);
struct tfs *close_fs(struct tfs *fs);
struct tfs *new_tfs(const char *fn, unsigned long sizeInBlocks, int numberOfInodes);

//write_to_fs.c
void writeDataBlock(struct tfs *fs, u8 *startAddress, u16 size);
void writeASCIILine(struct tfs *fs, u8** currentAddress);
void writeBootBlock(struct tfs* fs);
void writeSuperBlock(struct tfs* fs);
void writeZoneBMap(struct tfs* fs, int sizeInBlocks);
void writeInodeBMap(struct tfs* fs);
void writeInodes(struct tfs* fs);
void newline(struct tfs *fs);
void writeVirtualDataBlock(char *virtualFS, unsigned long zone,	u8 *startAddress, u16 size);
void writefile(struct tfs *fs, FILE *fp, int inode);
void writedata(struct tfs *fs, u8 *blk, u32 cnt, int inode);
void cmd_add(struct tfs *fs, int argc, char **argv);
void cmd_mklnk(struct tfs *fs,int argc,char **argv);
void cmd_hardlnk(struct tfs *fs,int argc,char **argv);

//read_from_fs.c
void printHeader(struct tfs *fs, unsigned long blockID);
void printBlock(struct tfs *fs, unsigned long blockID);
void print_bitmaps(struct tfs *fs);
void newline(struct tfs *fs);
void readZeroBlock(struct tfs *fs);
void readSuperBlock(struct tfs *fs);
void readFreeList(struct tfs *fs);
void readInodeList(struct tfs *fs);
void readInodes(struct tfs *fs);
void readVirtualDataBlock(char* BlockPtr, unsigned long address);
void readVirtualZoneBMap(struct tfs *fs);
void readVirtualInodeBMap(struct tfs *fs);
void readVirtualInodes(struct tfs* fs);
void readVirtualBootBlock(struct tfs *fs);
void readVirtualSuperBlock(struct tfs *fs);
void readHeaderWithDataBlock(struct tfs* fs, u8 *buf);
char *goto_dataBlk(char *virtualFS, unsigned long blk);
char *goto_Block(char *virtualFS, unsigned long blk);
void readVirtualFS(struct tfs *fs, const char *fn);
void cmd_readlink(struct tfs *fs,int argc,char **argv);
void cmd_cat(struct tfs *fs,int argc,char **argv);
void cmd_extract(struct tfs *fs,int argc,char **argv);

//spec_tfs.c
void initDefaultBlock(struct tfs *fs, unsigned long sizeInBlocks);
void initBootBlock(struct tfs *fs, unsigned long sizeInBlocks);
void initSuperBlock(struct tfs *fs, unsigned long sizeInBlocks);
void initBitmaps(struct tfs *fs, unsigned long sizeInBlocks, int numberOfInodes);
void initInodeTables(struct tfs *fs, unsigned long *rootblk);
void initRootBlock(struct tfs *fs, char *rootblk, unsigned long rootblkp);

//dir.c
void cmd_mkdir(struct tfs *fs, int argc, char **argv);
void cmd_dir(struct tfs *fs, int argc, char **argv);
void cmd_unlink(struct tfs *fs, int argc, char **argv);
void cmd_rmdir(struct tfs *fs, int argc, char **argv);
void cmd_stat(struct tfs *fs, int argc, char **argv);

//pentest.c
void TestFS(int argc, char **argv);

#endif /* PROTOS_H_ */
