/*
 * Copyright (C) 2016 - Christian Jürgens <christian.textfs@gmail.com>
 * Copyright (C) 2016 - Dirk Klingenberg
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


#include"protos.h"

/*
 * Test the TextFS binary
 * @argc	- from command line
 * @argv	- from command line
 * */
void TestFS(int argc, char **argv)
{
	printf("*************** Test TextFS *************************************************\n");

	printf("************* 1. Extend root directory start *************************************************\n");
	printf("create fs with size = 2,45 MB\n");
	printf("./TextFS TextFS.txt mkfs -i 10 -s 1000\n");
	argv[1] = "TextFS.txt";
	argv[2] = "mkfs";
	argv[3] = "-i";
	argv[4] = "20";
	argv[5] = "-s";
	argv[6] = "100";
	argc = 7;
	do_cmd(argc, argv);


	printf("\nCreate 16 directories, to extend the root directory");

	argv[2] = "mkdir";
	argv[3] = "/testdir1";
	argc = 4;
	do_cmd(argc, argv);

	argv[3] = "/testdir2";
	do_cmd(argc, argv);

	argv[3] = "/testdir3";
	do_cmd(argc, argv);

	argv[3] = "/testdir4";
	do_cmd(argc, argv);

	argv[3] = "/testdir5";
	do_cmd(argc, argv);

	argv[3] = "/testdir6";
	do_cmd(argc, argv);

	argv[3] = "/testdir7";
	do_cmd(argc, argv);

	argv[3] = "/testdir8";
	do_cmd(argc, argv);

	argv[3] = "/testdir9";
	do_cmd(argc, argv);

	argv[3] = "/testdir10";
	do_cmd(argc, argv);

	argv[3] = "/testdir11";
	do_cmd(argc, argv);

	argv[3] = "/testdir12";
	do_cmd(argc, argv);

	argv[3] = "/testdir13";
	do_cmd(argc, argv);

	argv[3] = "/testdir14";
	do_cmd(argc, argv);

	argv[3] = "/testdir15";
	do_cmd(argc, argv);

	argv[3] = "/testdir16";
	do_cmd(argc, argv);

	printf("\nDelete directory testdir15\n");
	argv[2] = "rmdir";
	argv[3] = "testdir15";
	do_cmd(argc, argv);

	printf("\nContent of root-directory\n");
	argv[2] = "dir";
	argv[3] = "/";
	do_cmd(argc, argv);

	printf("\nopen sfml\n");
	argv[2] = "sfml";
	do_cmd(argc, argv);

	printf("************* 1. Extend root directory finished *************************************************\n");

	printf("************* 2. sfml start *************************************************\n");
	printf("create fs with size = 2,45 MB\n");
	printf("./TextFS TextFS.txt mkfs -i 10 -s 1000\n");
	argv[1] = "TextFS.txt";
	argv[2] = "mkfs";
	argv[3] = "-i";
	argv[4] = "10";
	argv[5] = "-s";
	argv[6] = "100";
	argc = 7;
	do_cmd(argc, argv);


	printf("\nCreate some stuff to display with sfml");

	argv[2] = "mkdir";
	argv[3] = "/testdir1";
	argc = 4;
	do_cmd(argc, argv);

	argv[2] = "mkdir";
	argv[3] = "/testdir2";
	//argv[4] = "/";
	argc = 4;
	do_cmd(argc, argv);

	argv[2] = "mkdir";
	argv[3] = "/testdir3";
	//argv[4] = "/";
	argc = 4;
	do_cmd(argc, argv);

	printf("\nopen sfml\n");
	argv[2] = "sfml";
	do_cmd(argc, argv);
	printf("************* 2. sfml finished *************************************************\n");

//
//	FILE *fp;
//int i;
//	fp = fopen("dummy1", "w+b");
//	printf("\nadd dummy2 with size =  65000 * 512 B to root directory\n");
//	fp = fopen("dummy2", "w+b");
//
//
//
//	for (i = 0; i < 993 * BLOCKSIZE;i++)	//2089 geht ohne sfml
//	{
//		putc(1, fp);
//	}
//
//	fflush(fp);
//	argv[2] = "add";
//	argv[3] = "big_test_file.txt";
//	argv[4] = "/";
//	argc = 5;
//	do_cmd(argc, argv);
//	remove("dummy2");

//	printf("nopen sfml\n");
//	argv[2] = "sfml";
//	do_cmd(argc, argv);
	printf("\n*************End of Test*************************************************");
}
