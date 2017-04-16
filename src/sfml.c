/*
 * Copyright (C) 2016 - Christian Jürgens <christian.textfs@gmail.com>
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

#include "sfml.h"
#include "spec_tfs.h"

/*
 * contains all SFML stuff for displaying a window
 * @fs  			file system structure
 * */
void openWindow(struct tfs *fs)
{
	printf("Quit TextFS window to continue\n");

	sfWindowState window;
	sf_RenderWindow_myCreate(&window);

	sfInode sfInodes;
	sfButton inode[fs->sb->nInodes];

	sfInodes.inode = inode;
	sfInode_create(&sfInodes, fs, &window);

	// Start the window loop
	while (sfRenderWindow_isOpen(window.this))
	{
		sfPollEvent(&window, (sfInode *) &sfInodes, fs);

		if (window.active)
		{
			sfMoveScrollbar(window.scrollbar, &sfInodes);
			sfMoveScreen(&window);
			sfView_setCenter(window.view, window.viewPosition);

			sfInode_action(&window, (sfInode *) &sfInodes);

			// Render Window
			sfRenderWindow_clear(window.this, BACKGROUNDCOLOR);
			sfRenderWindow_drawInodes(&window.this, &sfInodes);
			sfRenderWindow_drawRectangleShape(window.this, window.scrollbar, NULL);
			sfRenderWindow_setView(window.this, window.view);
			sfRenderWindow_display(window.this);
		}
	}
	// Cleanup resources
	sfInode_destroy((sfInode *) &sfInodes);
	sfRenderWindow_myDestroy(&window);
}
