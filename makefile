# Copyright (C) 2016 - Dirk Klingenberg <blademountain35@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


LPATH = build/

OBJECTS = gen_tfs.o init_tfs.o iname.o inode.o penetration_test.o read_from_fs.o write_to_fs.o spec_tfs.o utils.o dir.o sf_functions.o sf_buttons.o sf_Inodes.o sfml.o main.o 

TextFS: $(OBJECTS) 
	gcc -L/sfml-build/lib -o TextFS $(OBJECTS) -lcsfml-graphics -lcsfml-window -lcsfml-system -lsfml-graphics -lsfml-window -lsfml-system

gen_tfs.o: src/gen_tfs.c
	gcc -c src/gen_tfs.c

init_tfs.o: src/init_tfs.c
	gcc -c src/init_tfs.c

iname.o: src/iname.c
	gcc -c src/iname.c

inode.o: src/inode.c
	gcc -c src/inode.c
	
penetration_test.o: src/penetration_test.c
	gcc -c src/penetration_test.c

read_from_fs.o: src/read_from_fs.c
	gcc -c src/read_from_fs.c

write_to_fs.o: src/write_to_fs.c
	gcc -c src/write_to_fs.c

spec_tfs.o: src/spec_tfs.c
	gcc -c src/spec_tfs.c

utils.o: src/utils.c
	gcc -c src/utils.c

dir.o: src/dir.c
	gcc -c src/dir.c

#-I<sfml-install-path>/include

sf_functions.o: src/sf_functions.c
	gcc -DCSFML-STATIC -c src/sf_functions.c -I/sfml/include

sf_buttons.o: src/sf_buttons.c
	gcc -DCSFML-STATIC -c src/sf_buttons.c -I/sfml/include

sf_Inodes.o: src/sf_Inodes.c
	gcc -DCSFML-STATIC -c src/sf_Inodes.c -I/sfml/include

sfml.o: src/sfml.c
	gcc -DCSFML-STATIC -c src/sfml.c  -I/sfml/include

main.o: src/main.c
	gcc -c src/main.c


