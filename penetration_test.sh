# Copyright (C) 2016 - Christian Jürgens <christian.textfs@gmail.com>
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
 
# run with "bash ./penetration_test.sh" on command line
 
#!/bin/sh

KEY="s"
function getsfml {
    if [ "$SFML" == "$KEY" ];then 
        printf "sfml (s) oder Eingabetaste zum fortsetzen "
        read COMMAND
        if [ "$COMMAND" == "$KEY" ];then
            ./TextFS TextFS.txt sfml /
        fi    
    fi
}

printf "*************test TextFS***********************************************"
printf "\nsfml ausgaben erzeugen (s) ? Eingabetaste zum fortsetzen "
read SFML


printf "\ncreate fs with size = 50000 kB"
printf "\n./TextFS TextFS.txt mkfs -i 10 -s 100000"
./TextFS TextFS.txt mkfs -i 10 -s 100000

printf "\n\nadd dummy1 with size = 7 * 512B to root directory\n"
dd if=/dev/urandom of=dummy1 bs=7 count=512
./TextFS TextFS.txt add dummy1 /
rm dummy1
getsfml

printf "\nshow dir"
./TextFS TextFS.txt dir /

printf "\nadd dummy2 with size =  7 * 512B + 256 * 512B to root directory\n"
dd if=/dev/urandom of=dummy2 bs=263 count=512
./TextFS TextFS.txt add dummy2 /
rm dummy2
getsfml

printf "\nshow dir"
./TextFS TextFS.txt dir /

printf "\nadd dummy3 with size =  7 * 512B + 256 * 512B + 14 * 256 * 512 to root directory\n"
dd if=/dev/urandom of=dummy3 bs=3847 count=512
./TextFS TextFS.txt add dummy3 /
rm dummy3
getsfml

printf "\nshow dir"
./TextFS TextFS.txt dir /

printf "\nadd dummy4 with size =  70kB to root directory\n"
dd if=/dev/urandom of=dummy4 bs=140 count=512
./TextFS TextFS.txt add dummy4 /
rm dummy4
getsfml

printf "\nshow dir"
./TextFS TextFS.txt dir /

printf "*************test TextFS***********************************************\n"
