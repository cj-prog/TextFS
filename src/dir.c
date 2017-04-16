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
 * Print a directory entry
 * @fp 			- file to write to (typically stdio)
 * @dirPtr	- pointer to directory entry's name
 * @namlen  - length of file name
 * */
void outent(FILE *fp, const char *dirPtr, int namlen)
{
	while (*dirPtr && namlen--)
	{
		putc(*dirPtr++, fp);
	}
	putc('\n', fp);
}

/*
 * List contents of a directory
 * @fs	 - file system structure
 * @path - directory path to list
 * */
void dodir(struct tfs *fs,const char *path)
{
  int inode = find_inode(fs,path);
  int i,bsz,j;
  u8 blk[BLOCKSIZE];
  int fdirsize;
  int dentsz = DIRSIZE(fs);

  if (inode == ERROR)
  {
  	fatalmsg("%s: not found",path);
  }

	struct tfs_inode *ino = INODE(fs,inode);

	if (!S_ISDIR(ino->i_mode))
	{
		fatalmsg("%s: is not a directory",path);
	}

	fdirsize = ino->i_size;

  for (i = 0; i < fdirsize; i += BLOCKSIZE)
  {
    bsz = read_inoblk(fs,inode,i / BLOCKSIZE,blk);
    for (j = 0; j < bsz ; j+= dentsz)
    {
      u16 fino = *((u16 *)(blk+j));

      if (!fino)
      {
      	continue;
      }
      outent(stdout,(char *)blk+j+2,dentsz-2);
    }
  }
}

/*
 * Commant to list contents of a directory
 * @fs		- file system structure
 * @argc  - from command line
 * @argv  - from command line
 * */
void cmd_dir(struct tfs *fs, int argc, char **argv)
{
	argv[2] = argv[3];
	argc = 2;

	if (argc == 1)
	{
		dodir(fs, ".");
	}
	else if (argc == 2)
	{
		dodir(fs, argv[2]);
	}
	else
	{
		int i;

		for (i = 1; i < argc; i++)
		{
			printf("%s:\n", argv[i]);
			dodir(fs, argv[i]);
		}
	}
}

/*
 * Create a directory
 * @fs			- file system structure
 * @newdir  - directory name
 * */
int domkdir(struct tfs *fs, char *newdir)
{
	int dinode;
	int ninode = make_node(fs, newdir, 0755 | S_IFDIR, 0, 0, 0,
												 NOW, NOW, NOW, &dinode);

	dname_add(fs, ninode, ".", ninode);
	dname_add(fs, ninode, "..", dinode);

	INODE(fs,dinode)->i_nlinks++;
	INODE(fs,ninode)->i_nlinks++;

	return ninode;
}

/*
 * Command to create directories
 * @fs - file system structure
 * @argc - from command line
 * @argv - from command line
 * */
void cmd_mkdir(struct tfs *fs, int argc, char **argv)
{
	int i;
	for (i = 3; i < argc; i++)
	{
		domkdir(fs, argv[i]);
	}
}

/*
 * Print time formatted
 * @fp 		- output file pointer
 * @str 	- type string
 * @usecs	- time stamp
 * */
void timefmt(FILE *fp, const char *str, u32 usecs)
{
	struct tm tmb;
	time_t secs = usecs;
	gmtime_r(&secs, &tmb);

	fprintf(fp, "\t%s=%04d/%02d/%02d %02d:%02d:%02d (GMT)\n",
					str,
					tmb.tm_year + 1900, tmb.tm_mon + 1, tmb.tm_mday,
					tmb.tm_hour,
					tmb.tm_min, tmb.tm_sec);
}

/*
 * Print formatted mode value
 * @fp 		- output file
 * @mode	- mode bits
 * */
void outmode(FILE *fp, int mode)
{
	switch (mode & S_IFMT)
	{
	case S_IFSOCK:
		fprintf(fp, "\ttype=socket\n");
		break;
	case S_IFLNK:
		fprintf(fp, "\ttype=symbolic link\n");
		break;
	case S_IFREG:
		fprintf(fp, "\ttype=regular file\n");
		break;
	case S_IFBLK:
		fprintf(fp, "\ttype=block device\n");
		break;
	case S_IFDIR:
		fprintf(fp, "\ttype=directory\n");
		break;
	case S_IFCHR:
		fprintf(fp, "\ttype=character device\n");
		break;
	case S_IFIFO:
		fprintf(fp, "\ttype=fifo\n");
		break;
	default:
		fprintf(fp, "\ttype=unknown (%o)\n", mode & S_IFMT);
	}
	fprintf(fp, "\tmode=%04o\n", mode & 07777);
}

/*
 * Print stat entries for a inode
 * @fs 		- file system structure
 * @path	- directory path to list
 * */
void dostat(struct tfs *fs,const char *path)
{
  int inode = find_inode(fs,path);

  if (inode == ERROR)
  {
    fprintf(stderr,"%s: not found\n",path);
    return;
  }

  printf("\tinode=%d\n",inode);

	struct tfs_inode *ino = INODE(fs,inode);
	outmode(stdout,ino->i_mode);
	printf("\tnlinks=%d\n",ino->i_nlinks);

	if (S_ISCHR(ino->i_mode) || S_ISBLK(ino->i_mode))
	{
		printf("\tmajor=%d\nminor=%d\n",
					(ino->zones[0]>>8) & 0xff,
					 ino->zones[0] & 0xff);
	}
	else
	{
		printf("\tsize=%d\n",ino->i_size);
	}
	timefmt(stdout,"accessed",ino->i_atime);
}

/*
 * Command to stat entries
 * @fs 		- file system structure
 * @argc	- from command line
 * @argv 	- from command line
 * */
void cmd_stat(struct tfs *fs, int argc, char **argv)
{
	if (argc == 1)
	{
		dostat(fs, ".");
	}
	else if (argc == 2)
	{
		dostat(fs, argv[1]);
	}
	else
	{
		int i;

		for (i = 3; i <= argc; i++)
		{
			printf("%s:\n", argv[i]);
			dostat(fs, argv[i]);
		}
	}
}

/*
 * Remove an empty directory
 * @fs 	- file system structure
 * @dir	- directory to remove
 * */
void dormdir(struct tfs *fs,const char *dir)
{
  int inode = find_inode(fs,dir);
  int i,bsz,j;
  u8 blk[BLOCKSIZE];
  int fdirsize;
  int dentsz = DIRSIZE(fs);
  int pinode = -1;
  const char *p;

  if (inode == ERROR)
  {
  	fatalmsg("%s: not found",dir);
 	}
  else if (inode == TFS_ROOT_INO)
  {
  	fatalmsg("Can not remove root inode");
	}

  // Make sure directory is a directory
	struct tfs_inode *ino = INODE(fs,inode);

	if (!S_ISDIR(ino->i_mode))
	{
		fatalmsg("%s: is not a directory",dir);
	}

	fdirsize = ino->i_size;

  // Do a directory scan...
  for (i = 0; i < fdirsize; i += BLOCKSIZE)
  {
    bsz = read_inoblk(fs,inode,i / BLOCKSIZE,blk);

    for (j = 0; j < bsz ; j+= dentsz)
    {
      u16 fino = *((u16 *)(blk+j));

      if (blk[j+2] == '.' && blk[j+3] == 0)
      {
      	continue;
      }
      else if (blk[j+2] == '.' && blk[j+3] == '.'&& blk[j+4] == 0)
      {
        pinode = fino;
        continue;
      }
      if (fino != 0)
      {
      	fatalmsg("%s: not empty",dir);
      }
    }
  }

  // Free stuff
  trunc_inode(fs,inode,0);
  clr_inode(fs, inode);
  INODE(fs,pinode)->i_nlinks--;
  p = strrchr(dir,'/');

  if (p)
  {
    p++;
  }
  else
  {
    p = dir;
  }

  dname_rem(fs,pinode,p);
}

/*
 * Remove directories
 * @param fs - file system structure
 * @param argc - from command line
 * @param argv - from command line
 * */
void cmd_rmdir(struct tfs *fs, int argc, char **argv)
{
	int i;
	for (i = 3; i < argc; i++)
	{
		dormdir(fs, argv[i]);
	}
}

/*
 * Remove an file (not directory)
 * @fs 		- file system structure
 * @fpath	- file path
 * */
void dounlink(struct tfs *fs, char *fpath)
{
	char *dir = fpath;
	char *fname = strrchr(fpath, '/');
	int dinode, inode;
	char dir_string[64];
	char *dir_ptr;

	strcpy(dir_string, dir);

	if (fname)
	{
		fname++;
		dir_ptr = strrchr(dir_string, '/');
		*dir_ptr = '\0';
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

	inode = ilookup_name(fs, dinode, fname, NULL, NULL);

	if (inode == ERROR)
	{
		fatalmsg("%s: not found\n", fname);
	}

	// Make sure file is not a directory
	struct tfs_inode *ino = INODE(fs, inode);

	if(ino->i_mode == 16877)
	{
		fatalmsg("%s: is a directory", dir);
	}

	dname_rem(fs, dinode, fname);

	if (--(ino->i_nlinks))
	{
		return;
	}

	// Remove stuff...
	trunc_inode(fs, inode, 0);
	clr_inode(fs, inode);
}

/*
 * Remove files (but not directories)
 * @fs    - file system structure
 * @argc  - from command line
 * @argv	- from command line
 * */
void cmd_unlink(struct tfs *fs, int argc, char **argv)
{
	int i;

	for (i = 3; i < argc; i++)
	{
		dounlink(fs, argv[i]);
	}
}
