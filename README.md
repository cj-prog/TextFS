# TextFS

TextFS - The file system in a text file. 


## 

TextFS based on the Minix File System and provide several system calls like mkfs, mkdir, add and dir, as well it visualizes the structure of the file system in a window based CSFML tool. 

Example file system: [TextFS.txt](https://github.com/cj-prog/TextFS/blob/master/examples/TextFS.txt).

## Authors

Christian Jürgens - main developer (christian.textfs@gmail.com)                                                               
Dirk Klingenberg  - main developer (blademountain35@gmail.com)

## Mentor

Prof. Dr. Carsten Link,                                                                                                       
University of Applied Sciences Emden/Leer                                                                                     
Faculty of Technology

## Download

You can get the official release from the [Git repository](https://github.com/cj-prog/TextFS).

## Install

Unpack the downloaded file to your preferred location, e.g. ~/home/user/workspace and run

         ~/home/user/workspace/TextFS$ sudo sh setup.sh
	
This shell script installs needed dependencies and auto-generate files for SFML on Ubuntu. For other distribution the content need to be adjusted. For problems with SFML [http://www.sfml-dev.org/faq.php#build-use](http://www.sfml-dev.org/faq.php#build-use) maybe helpful.

## Usage

For usage instructions just type:
        
         ./TextFS   
	 
## Example

Simple example create file system and visualize it:

         ./TextFS TextFS.txt mkfs
         ./TextFS TextFS.txt sfml	
   
   
## Credits

[SFML](http://www.sfml-dev.org/)                                                                                             
[mfs-tool](https://sourceforge.net/projects/mfstool/)
