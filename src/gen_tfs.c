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
#include <getopt.h>
#include "spec_tfs.h"

/*
 * Parse mkfs/genfs command line arguments
 * @argc		 	- from command line
 * @argv 			- from command line
 * @nblks_p		- returns size of file system
 * @inodes_p	- return number of requested inodes
 *
 * -i nodecount
 * -s nblocks
 * */
void get_size_parameters(int argc, char **argv, unsigned long *nblks_p, int *inodes_p)
{
	*nblks_p = -1;
  *inodes_p = 0;

//  int c;
//  while (1) {
//    //c = getopt(argc,argv,"12vi:n:s:");
//  	c = getopt(argc,argv,"i:s:");
//    if (c == -1) break;
//    switch (c) {
//    case 'i':
//      *inodes_p = atoi(optarg);
//      break;
//    case 's':
//      *nblks_p = atoi(optarg);
//      break;
//    }
//  }

  //Get parameters without getopt (for MAC OS X)
	int i;

	for (i = 3; i < argc; i++ )
	{
		if(!strcmp(argv[i], "-i"))
		{
			*inodes_p = atoi(argv[i+1]);
			if(*inodes_p < MINIMUM_INODES)
			{
				*nblks_p = MINIMUM_INODES;
				printf("\nMinimum size of inodes is %d\n",MINIMUM_INODES);
				printf("Number of Inodes set to 1\n\n");
			}
			else if(*inodes_p > MAXIMUM_INODES)
			{
				*nblks_p = -1;
				i = argc;
				printf("\nMaximum size of inodes is %d\n",MAXIMUM_INODES);
				exit(0);
			}
		}
		else if(!strcmp(argv[i], "-s"))
		{
			*nblks_p = atoi(argv[i+1]);
			if(*nblks_p < MINIMUM_BLOCKS)
			{
				*nblks_p = MINIMUM_BLOCKS;
				printf("\nMinimum size of blocks is %d\n", MINIMUM_BLOCKS);
				printf("Number of Blocks set to 30\n\n");
			}
			else if(*nblks_p > MAXIMUM_BLOCKS)
			{
				*nblks_p = -1;
				i = argc;
				printf("\nMaximum size of blocks is %d\n",MAXIMUM_BLOCKS);
				exit(0);
			}
		}
		i++;
	}

  if (*nblks_p == -1)
  {
  	printf("\nno file system size specified\n\n%s mkfs -i [number of inodes] -s [number of blocks]\n", argv[1]);
		*inodes_p = DEFAULT_INODES;
		*nblks_p = DEFAULT_BLOCKS;
		printf("\nDefault Values are set to -i %d -s %d\n\n", DEFAULT_INODES, DEFAULT_BLOCKS);
  }
}

/*
 * Create an empty file system
 * @argc - from command line
 * @argv - from command line
 * */
void cmd_mkfs(int argc,char **argv)
{
  int numberOfInodes = 0;
  unsigned long sizeInBlocks = 0;
  char *target_filename = argv[1];

  get_size_parameters(argc,argv, &sizeInBlocks, &numberOfInodes);
  new_tfs(target_filename,sizeInBlocks,numberOfInodes);
}
