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
 * show command usage
 * @name	- TextFS binary
 * */
void generalUsage(const char* name)
{
	printf("\nUsage: %s --version \t show version of filesystem\n", name);
	printf("\nUsage: %s --copyright \t show copyright\n", name);
	printf("\nUsage: %s [fs-name.txt] [command] {optfile} \n\n", name);
	printf("Commands:\n");
	printf("mkfs \t\t make new file system\n");
	printf("mkdir \t\t make new directory\n");
	printf("add \t\t add file\n");
	printf("dir \t\t show directory\n");
	printf("unlink \t\t removes one or more files\n");
	printf("rmdir \t\t removes one or more directories\n");
	printf("stat \t\t show details of file \n");
	printf("symlink \t create a symlink to file\n");
	printf("hardlink \t create a hardlink to file \n");
	printf("readlink \t show the target file from symlink \n");
	printf("cat \t\t show content of file in console \n");
	printf("extract \t extract a file from file system \n");
	printf("sfml \t\t open new window and show details of inode structure\n\n");

	exit(0);
}

/*
 * show global information
 * @argv	- from command line
 * */
void globalOpts(char **argv)
{
	if(!strcmp(argv[1], "--help"))
	{
		generalUsage(argv[0]);
	}
	else if (!strcmp(argv[1], "--version"))
	{
		printf("\nFilesystem-Version: %s\n\n", VERSION);
		exit(0);
	}
	else if (!strcmp(argv[1], "--copyright"))
	{
		printf("\n============================================================================\n"
				" * Copyright (C) 2016 - Christian Jürgens <christian.textfs@gmail.com>\n"
				" * Copyright (C) 2016 - Dirk Klingenberg <blademountain35@gmail.com>\n"
				" * \n"
				" * This program is free software; you can redistribute it and/or modify\n"
				" * it under the terms of the GNU General Public License as published by\n"
				" * the Free Software Foundation; either version 2 of the License, or\n"
				" * (at your option) any later version.\n"
				" *\n"
				" * This program is distributed in the hope that it will be useful,\n"
				" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
				" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
				" * GNU General Public License for more details.\n"
				" *\n"
				" * You should have received a copy of the GNU General Public License\n"
				" * along with this program; if not, write to the Free Software\n"
				" * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
				"============================================================================\n\n");
		exit(0);
	}
}

/*
 * show specific usage information
 * @name	- TextFS binary
 * @opt		-	command
 * */
void usage(const char* name, const char* opt)
{
	if (!strcmp(opt, "mkdir"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [directory-name] \n\n", name, opt);
	}
	else if (!strcmp(opt, "add"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [sourcepath] [targetpath] \n", name, opt);
		printf("\nAdd to root:");
		printf("\nExample: %s [fs-name.txt] %s [path/filename] [/] \n", name, opt);
		printf("\nAdd to directory:");
		printf("\nExample: %s [fs-name.txt] %s [path/filename] [directory/] \n\n", name, opt);
	}
	else if (!strcmp(opt, "dir"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [/directory-name] \n", name, opt);
		printf("If you want to see the root directory, then use \"/\" only.\n\n");
	}
	else if (!strcmp(opt, "unlink"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [directory/first_file] \n", name, opt);
		printf( "If you want to remove more then one file, then supplement the usage "
						"with [directory/second_file].\n\n");
	}
	else if (!strcmp(opt, "rmdir"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [first_directory] \n", name, opt);
		printf(	"If you want to remove more then one directory, then supplement the usage "
						"with [second_directory].\n\n");
	}
	else if (!strcmp(opt, "stat"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [file] \n", name, opt);
	}
	else if (!strcmp(opt, "symlink") || !strcmp(opt, "hardlink"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [directory/link target] [directory/link name]  \n\n", name, opt);
	}
	else if (!strcmp(opt, "readlink"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [file] \n\n", name, opt);
	}
	else if (!strcmp(opt, "cat"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [file] \n\n", name, opt);
	}
	else if (!strcmp(opt, "extract"))
	{
		printf("\nUsage: %s [fs-name.txt] %s [sourcepath] [targetpath] \n\n", name, opt);
	}
	else if (!strcmp(opt, "sfml"))
	{
		printf("\nUsage: %s [fs-name.txt] %s \n\n", name, opt);
	}
	else
	{
		generalUsage(name);
	}
	exit(0);
}

/*
 * so command
 * @argc	- from command line
 * @argv	-	from command line
 * */
void do_cmd(int argc, char **argv)
{
	globalOpts(argv);

	if(argc < 3)
	{
		generalUsage(argv[0]);
	}

	if (!strcmp(argv[2], "mkfs"))
	{
		cmd_mkfs(argc, argv);
	}
	else if (!strcmp(argv[2], "sfml"))
	{
		struct tfs *fs = open_fs(argv[1]);
		openWindow(fs);
	}
	else if (!strcmp(argv[2], "pentest"))
	{
		printf("pentest\n");
		TestFS(argc, argv);
	}
	else
	{
		if (argc < 4)
			usage(argv[0], argv[2]);

		struct tfs *fs = open_fs(argv[1]);

		if (!strcmp(argv[2], "dir"))
		{
			cmd_dir(fs, argc, argv);
		}
		else if (!strcmp(argv[2], "mkdir"))
		{
			cmd_mkdir(fs, argc, argv);
		}
		else if (!strcmp(argv[2], "rmdir"))
		{
			cmd_rmdir(fs,argc,argv);
		}
		else if (!strcmp(argv[2], "unlink"))
		{
			cmd_unlink(fs, argc, argv);
		}
		else if (!strcmp(argv[2], "cat"))
		{
			cmd_cat(fs,argc,argv);
		}
		else if (!strcmp(argv[2], "extract"))
		{
			if(argc < 5)
				usage(argv[0], argv[2]);
			cmd_extract(fs,argc,argv);
		}
		else if (!strcmp(argv[2], "readlink"))
		{
			cmd_readlink(fs,argc,argv);
		}
		else if (!strcmp(argv[2], "symlink"))
		{
			if(argc < 5)
				usage(argv[0], argv[2]);
			cmd_mklnk(fs,argc,argv);
		}
		else if (!strcmp(argv[2], "hardlink"))
		{
			if(argc < 5)
				usage(argv[0], argv[2]);
			cmd_hardlnk(fs,argc,argv);
		}
		else if (!strcmp(argv[2], "stat"))
		{
			cmd_stat(fs,argc,argv);
		}
		else if (!strcmp(argv[2], "add"))
		{
			if(argc < 5)
				usage(argv[0], argv[2]);
			cmd_add(fs, argc, argv);
		}
		close_fs(fs);
	}
}

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		argv[1] ="--help";
	}
	do_cmd(argc, argv);
	return EXIT_SUCCESS;
}
