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

/*
 * Create window
 * @window	-	sfWindowState structure
 * */
void sf_RenderWindow_myCreate(sfWindowState *window)
{
	// Initialize default window
	window->updateView = TRUE;
	window->active = TRUE;
	window->scale = 1;
	window->resize_offset.x = 0;
	window->resize_offset.y = 0;
	window->viewPosition.x = SCREENWIDTH * 0.5;
	window->viewPosition.y = SCREENHEIGHT * 0.5;
	window->view = sfView_create();

	sfVideoMode mode = { SCREENWIDTH, SCREENHEIGHT, BITSPERPIXEL };

	window->this = sfRenderWindow_create(mode, "TextFS Visualizer",
																				sfResize | sfClose, NULL);

	sfWindow_setPosition((sfWindow*) window->this, sfVector2i_set(0, 0));

	// Set scrollbar
	window->scrollbar = sfRectangleShape_create();
	sfRectangleShape_setFillColor(window->scrollbar, sfBlack);
	sfRectangleShape_setSize( window->scrollbar,
													 	sfVector2f_set(SCROLLBARSIZE, SCROLLBARSIZE));

	sfRectangleShape_setPosition( window->scrollbar,
																sfVector2f_set(SCREENWIDTH - SCROLLBARSIZE, 0));
}

/*
 * Destroy window
 * @window	-	window structure
 * */
void sfRenderWindow_myDestroy(sfWindowState *window)
{
	sfView_destroy(window->view);
	sfRectangleShape_destroy(window->scrollbar);
	sfRenderWindow_destroy(window->this);
}

/*
 * Create an empty array for lines
 * @return	- line array
 * */
sfVertexArray *sfVertexArray_createLine()
{
	sfVertexArray *line = sfVertexArray_create();
	sfVertexArray_setPrimitiveType(line, SF_LINE);

	return line;
}

/*
 * Fill array with a line between buttons
 * @line				- line array
 * @fromSprite	- begin of line
 * @toSprite		- end of line
 * */
void sfVertexArray_setLine( sfVertexArray *line, sfSprite *fromSprite,
																								 sfSprite *toSprite)
{
	sfVertexArray_clear(line);

	// Set begin of line
	sfVertexArray_append( line,
												sfVertex_set(sfSprite_getPosition(fromSprite).x
																		 + sfSprite_getGlobalBounds(fromSprite).width,
																		 sfSprite_getPosition(fromSprite).y
																		 + sfSprite_getGlobalBounds(fromSprite).height * 0.5,
																		 LINECOLOR, 0, 0));
	// Set end of line
	sfVertexArray_append( line,
												sfVertex_set(sfSprite_getPosition(toSprite).x,
																		 sfSprite_getPosition(toSprite).y
																		 + sfSprite_getGlobalBounds(toSprite).height * 0.5,
																		 LINECOLOR, 0, 0));
}

/*
 * Fill array with a line between button and text
 * @line				- line array
 * @fromSprite	- begin of line
 * @toSprite		- end of line
 * */
void sfVertexArray_setBlockLine(sfVertexArray *blockLine, sfSprite *fromSprite,
																													sfSprite *toSprite)
{
	sfVertexArray_clear(blockLine);

	// Set begin of line
	sfVertexArray_append( blockLine,
												sfVertex_set( sfSprite_getPosition(fromSprite).x
																			+ sfSprite_getGlobalBounds(fromSprite).width,
																			sfSprite_getPosition(fromSprite).y
																			+ sfSprite_getGlobalBounds(fromSprite).height * 0.5,
																			LINECOLOR, 0, 0));

	// Set end of line
	sfVertexArray_append( blockLine,
												sfVertex_set(	sfSprite_getPosition(toSprite).x + XSPACE,
																			sfSprite_getPosition(toSprite).y
																			+ sfSprite_getGlobalBounds(toSprite).height	* 0.5,
																			LINECOLOR, 0, 0));
}

/*
 * Set vertex position
 * @x				-	x position
 * @y				-	y position
 * @return	- vertex
 * */
sfVertex sfVertex_setPos(float x, float y)
{
	sfVertex vertex;

	vertex.position.x = x;
	vertex.position.y = y;

	return vertex;
}

/*
 * Set vertex values
 * @x				-	x position
 * @y				-	y position
 * @color		- vertex color
 * @tx			- x position of texture
 * @ty			- y position of texture
 * @return	- vertex
 * */
sfVertex sfVertex_set(float x, float y, sfColor color, float tx, float ty)
{
	sfVertex vertex;

	vertex.position.x = x;
	vertex.position.y = y;
	vertex.color = sfBlue;
	vertex.texCoords.x = tx;
	vertex.texCoords.y = ty;

	return vertex;
}

/*
 * Set 2-dim float vector
 * @x				-	x position
 * @y				-	y position
 * @return	- initialized float vector
 * */
sfVector2f sfVector2f_set(float x, float y)
{
	sfVector2f vector2f;

	vector2f.x = x;
	vector2f.y = y;

	return vector2f;
}

/*
 * Set 2-dim unsigned vector
 * @x				-	x position
 * @y				-	y position
 * @return	- initialized unsigned vector
 * */
sfVector2u sfVector2u_set(unsigned int x, unsigned int y)
{
	sfVector2u vector2u;

	vector2u.x = x;
	vector2u.y = y;

	return vector2u;
}

/*
 * Set 2-dim int vector
 * @x				-	x position
 * @y				-	y position
 * @return	- initialized int vector
 * */
sfVector2i sfVector2i_set(int x, int y)
{
	sfVector2i vector2i;

	vector2i.x = x;
	vector2i.y = y;

	return vector2i;
}

/*
 * Set rectangle values
 * @top			- upper positen
 * @left		- left position
 * @width		- width of rectangle
 * @height	- height of rectangle
 * @return	- initialized rectangle
 * */
sfFloatRect sfFloatRect_set(float top, float left, float width, float height)
{
	sfFloatRect floatRect;

	floatRect.top = top;
	floatRect.left = left;
	floatRect.width = width;
	floatRect.height = height;

	return floatRect;
}

/*
 * Checks arrow keys and move scrollbar
 * @scrollbar	-	scrollbar
 * @sfInodes	-	sfInodes structure
 * */
void sfMoveScrollbar(sfRectangleShape* scrollbar, sfInode *sfInodes)
{
	if (sfKeyboard_isKeyPressed(sfKeyUp))
	{
		if (sfRectangleShape_getPosition(scrollbar).y	> sfText_getPosition(sfInodes->info).y)
		{
				sfRectangleShape_move(scrollbar, sfVector2f_set(0, -SCROLLBARSIZE));
		}
	}
	else if (sfKeyboard_isKeyPressed(sfKeyLeft))
	{
		if (sfRectangleShape_getPosition(scrollbar).x
				> sfSprite_getPosition(sfInodes->indirZone->sprite).x)
		{
			sfRectangleShape_move(scrollbar, sfVector2f_set(-SCROLLBARSIZE, 0));
		}
	}
	else if (sfKeyboard_isKeyPressed(sfKeyRight))
	{
		sfRectangleShape_move(scrollbar, sfVector2f_set(SCROLLBARSIZE, 0));
	}
	else if (sfKeyboard_isKeyPressed(sfKeyDown))
	{
		sfRectangleShape_move(scrollbar, sfVector2f_set(0, SCROLLBARSIZE));
	}
}

/*
 * Move screen
 * @window	-	window structure
 * */
void sfMoveScreen(sfWindowState *window)
{
	window->viewPosition.x = 	sfRectangleShape_getPosition(window->scrollbar).x
														+ sfRectangleShape_getGlobalBounds(window->scrollbar).width
														- sfView_getSize(window->view).x * 0.5;

	window->viewPosition.y = 	sfRectangleShape_getPosition(window->scrollbar).y
														+ sfView_getSize(window->view).y * 0.5;
}

/*
 * Set new positions on resize event
 * @window		-	window structure
 * @sfInodes	- sfInodes structure
 * @fs				- file system structure
 * @sfEvent		- window event
 * */
void sfView_Event(sfWindowState *window, sfInode *sfInodes, struct tfs *fs,	sfEvent event)
{
	int active;

	// Several loops in predefined order to prevent render errors

	for (active = 0; active < fs->sb->nInodes; active++)
	{
		sfButton_setBox(&sfInodes->inode[active],
										(char *) sfText_getString(sfInodes->inode[active].text),
										X0(window) + INODE_X_POS,
										Y0(window) + INODE_Y_POS + active * YSPACE);
	}

	for (active = 0; active < NR_OF_ZONES; active++)
	{
		sfButton_setLinedBox(&sfInodes->zone[active],
												 sfInodes->inode[sfInodes->activeInode].sprite,
												 (char *) sfText_getString(sfInodes->zone[active].text),
												 (int) (INODEPOS(sfInodes).x + XSPACE),
												 (int) (INODEPOS(sfInodes).y + YSPACE * 0.5 + YSPACE * active));
	}

	for (active = 0; active < NR_OF_EXPANDS; active++)
	{
		sfButton_setLinedBox(	&sfInodes->expand1[active],
													sfInodes->zone[sfInodes->activeZone].sprite,
													(char *) sfText_getString(sfInodes->expand1[active].text),
													(int) (INODEPOS(sfInodes).x + 2 * XSPACE),
													(int) (INODEPOS(sfInodes).y	+ YSPACE * active * 0.6));
	}

	for (active = 0; active < NR_OF_EXPANDS; active++)
	{
		sfButton_setLinedBox(	&sfInodes->expand2[active],
													sfInodes->expand1[sfInodes->activeExpand1].sprite,
													(char *) sfText_getString(sfInodes->expand2[active].text),
													(int) (INODEPOS(sfInodes).x + 3 * XSPACE),
													(int) (INODEPOS(sfInodes).y + YSPACE * active * 0.6));
	}

	for (active = 0; active < NR_OF_EXPANDS; active++)
	{
		sfButton_setLinedBox(	&sfInodes->expand3[active],
													sfInodes->expand2[sfInodes->activeExpand2].sprite,
													(char *) sfText_getString(sfInodes->expand3[active].text),
													(int) (INODEPOS(sfInodes).x + 4 * XSPACE),
													(int) (INODEPOS(sfInodes).y + YSPACE * active * 0.6));
	}

	for (active = 0; active < NR_OF_EXPANDS; active++)
	{
		if (sfInodes->activeZone == INDIRZONE)
		{
			sfButton_setLinedBox(	&sfInodes->indirZone[active],
														sfInodes->expand1[sfInodes->activeExpand1].sprite,
														(char *) sfText_getString(sfInodes->indirZone[active].text),
														(int) (INODEPOS(sfInodes).x + 3 * XSPACE),
														(int) (INODEPOS(sfInodes).y + YSPACE * active * 0.6));
		}
		else if (sfInodes->activeZone == DOUBLEINDIRZONE)
		{
			sfButton_setLinedBox(	&sfInodes->indirZone[active],
														sfInodes->expand3[sfInodes->activeExpand3].sprite,
														(char *) sfText_getString(sfInodes->indirZone[active].text),
														(int) (INODEPOS(sfInodes).x + 5 * XSPACE),
														(int) (INODEPOS(sfInodes).y + YSPACE * active * 0.6));
		}
	}

	if (sfInodes->activeZone < INDIRZONE)
	{
		sfText_setPosition(	sfInodes->block,
												sfVector2f_set(sfSprite_getPosition(sfInodes->zone[0].sprite).x
																			 + XSPACE,
																			 INODEPOS(sfInodes).y));

		sfVertexArray_setBlockLine( sfInodes->blockLine,
																sfInodes->zone[sfInodes->activeZone].sprite,
																sfInodes->zone[0].sprite);
	}
	else
	{
		sfText_setPosition( sfInodes->block,
												sfVector2f_set(sfSprite_getPosition(sfInodes->indirZone[0].sprite).x
																			 + XSPACE,
																			 INODEPOS(sfInodes).y));

		sfVertexArray_setBlockLine(	sfInodes->blockLine,
																sfInodes->indirZone[sfInodes->activeIndirZone].sprite,
																sfInodes->indirZone[0].sprite);
	}

	sfText_setPosition(sfInodes->info, sfVector2f_set(X0(window), Y0(window)));
	sfText_setPosition( sfInodes->indirInfo,
											sfVector2f_set( sfText_getPosition(sfInodes->info).x,
																			sfText_getPosition(sfInodes->info).y
																			+ INFOLINES * CHARSIZE));

	sfText_setPosition( sfInodes->doubleIndirInfo,
											sfVector2f_set(	sfText_getPosition(sfInodes->info).x,
																			sfText_getPosition(sfInodes->info).y
																			+ INDIRINFOLINES * CHARSIZE));

	window->resize_offset.x = (int)  (event.size.width  - SCREENWIDTH)  * 0.5;
	window->resize_offset.y = (int) -(event.size.height - SCREENHEIGHT) * 0.5;

	sfRectangleShape_setPosition( window->scrollbar,
																sfVector2f_set(window->resize_offset.x + SCROLLBARPOS,
																window->resize_offset.y));

	sfView_reset(window->view, sfFloatRect_set(0, 0, event.size.width, event.size.height));
}

/*
 * poll user triggered events
 * @window		-	window structure
 * @sfInodes	- sfInodes structure
 * @fs				-	file system structure
 * */
void sfPollEvent(sfWindowState *window, sfInode *sfInodes, struct tfs *fs)
{
	sfEvent event;

	if (window->updateView)
	{
		window->updateView = FALSE;
		event.size.width	 = sfRenderWindow_getSize(window->this).x;
		event.size.height  = sfRenderWindow_getSize(window->this).y;
		sfView_Event(window, sfInodes, fs, event);
	}

	while (sfRenderWindow_pollEvent(window->this, &event))
	{
		if (event.type == sfEvtClosed)
		{
			sfRenderWindow_close(window->this);
		}
		else if (event.type == sfEvtLostFocus)
		{
			window->active = FALSE;
		}
		else if (event.type == sfEvtGainedFocus)
		{
			window->active = TRUE;
		}

		if (event.mouseButton.type == sfEvtMouseButtonReleased)
		{
			sfInode_Event(sfInodes, fs);
		}
		if (event.size.type == sfEvtResized)
		{
			sfView_Event(window, sfInodes, fs, event);
		}
	}
}

/*
 * Create default text
 * @text					-	text structure
 * @string				- string to be set
 * @font					-	font to be set
 * @characerSize	- size of characters
 * @x							- text x position
 * @y							-	text y position
 * */
void sfText_mycreate( sfText **text, char *string, sfFont **font,	int characterSize,
											int xPos, int yPos)
{
	*text = sfText_create();

	sfText_setString(*text, (const char *) string);
	sfText_setFont(*text, *font);
	sfText_setCharacterSize(*text, characterSize);
	sfText_setPosition(*text, sfVector2f_set(xPos, yPos));

}

/*
 * Set text of block
 * @text					-	text structure
 * @fs						- file system
 * @blockID				- block id for text
 * */
void sfText_setBlockString(sfText* text, struct tfs *fs, unsigned long blockID)
{
	int size = get_block_size(fs, blockID);
	char block[size + 1];

	get_block(block, fs, blockID, size);
	sfText_setString(text, (const char *) block);
}
