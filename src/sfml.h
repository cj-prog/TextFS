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

/*
 * includes, constants, structures and prototypes to visualize with SFML
 * */

#ifndef SFML_H_
#define SFML_H_

#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <stdlib.h>
#include "protos.h"

#define TRUE 1
#define FALSE 0
#define SCREENWIDTH 1500
#define SCREENHEIGHT 900
#define BITSPERPIXEL 32
#define BACKGROUNDCOLOR sfBlack
#define LINECOLOR sfRed
#define TEXTFONT "sfmlStuff/DejaVuSansMono.ttf"
#define SCROLLBARSIZE 20
#define BUTTONTEXTURE "sfmlStuff/buttontexture.png"
#define BUTTONHEIGHT 20
#define BUTTONWIDTH BUTTONHEIGHT * 2.5
#define MAXBUTTONCHARS 10
#define INODE_X_POS	400
#define INODE_Y_POS	90
#define XSPACE 2 * BUTTONWIDTH
#define YSPACE 2 * BUTTONHEIGHT
#define SF_LINE 2
#define CHARSIZE 10
#define INFOCHARSIZE CHARSIZE -2
#define NR_OF_ZONES 9
#define INDIRZONE NR_OF_ZONES - 2
#define DOUBLEINDIRZONE NR_OF_ZONES - 1
#define NR_OF_EXPANDS 16
#define INFOLINES 20
#define INDIRINFOLINES 52
#define SCROLLBARPOS SCREENWIDTH - SCROLLBARSIZE

#define TOGGLE_BUTTON(button)	button.isDebounced = FALSE, button.isClicked = !button.isClicked;
#define X0(window) window->viewPosition.x - sfWindow_getSize((sfWindow *)window->this).x * 0.5
#define Y0(window) window->viewPosition.y - sfWindow_getSize((sfWindow *)window->this).y * 0.5
#define	INODEPOS(sfInodes) sfSprite_getPosition(sfInodes->inode[sfInodes->activeInode].sprite)

/*
 * Default button configuration
 * */
typedef struct
{
	sfTexture* texture;
	sfIntRect intRect;
	sfSprite* sprite;
	sfText* text;
	sfFont* font;
	int isClicked;
	int isDebounced;
	sfVertexArray *line;
} sfButton;

/*
 * Contains every displayed item and its state information
 * */
typedef struct
{
	sfFont *font;
	sfButton *inode;
	sfButton expand1[NR_OF_EXPANDS];
	sfButton expand2[NR_OF_EXPANDS];
	sfButton expand3[NR_OF_EXPANDS];
	sfButton zone[NR_OF_EXPANDS];
	sfButton indirZone[NR_OF_EXPANDS];
	sfText *block;
	sfText* info;
	sfText* indirInfo;
	sfText* doubleIndirInfo;
	int numberOfInodes;
	int activeInode;
	int activeZone;
	int activeExpand1;
	int activeExpand2;
	int activeExpand3;
	int activeIndirZone;
	sfVertexArray *blockLine;
} sfInode;

/*
 * Default window configuration
 * */
typedef struct
{
	sfRenderWindow* this;
	int active;
	sfView *view;
	sfVector2f viewPosition;
	sfRectangleShape* scrollbar;
	sfVector2i resize_offset;
	float scale;
	int updateView;
} sfWindowState;

//sf_functions.c prototypes
void sf_RenderWindow_myCreate( sfWindowState *window);
void sfRenderWindow_myDestroy(sfWindowState *window);
sfVector2f sfVector2f_set( float x, float y);
sfVector2u sfVector2u_set( unsigned int x, unsigned int y);
sfVector2i sfVector2i_set(int x, int y);
sfFloatRect sfFloatRect_set( float top, float left, float width, float height);
sfVertex sfVertex_set( float x, float y, sfColor color, float tx, float ty);
void sfMoveScrollbar( sfRectangleShape* scrollbar, sfInode *sfInodes);
void sfMoveScreen( sfWindowState* window);
void sfPollEvent( sfWindowState *window, sfInode *sfInodes, struct tfs *fs);
void sfText_mycreate(	sfText **text, char *string, sfFont **font,
											int characterSize, int xPos, int yPos);
sfVertexArray *sfVertexArray_createLine();
void sfVertexArray_setLine(	sfVertexArray *line, sfSprite *fromSprite,
																			sfSprite *toSprite);
void sfVertexArray_setBlockLine( sfVertexArray *blockLine,	sfSprite *fromSprite,
																 sfSprite *toSprite);
void sfText_setBlockString(sfText* text, struct tfs *fs, unsigned long blockID);

//sf_buttons.c prototypes
void sfButton_create(sfButton *button, char *buttonText, int width, int height);
void sfButton_destroy(sfButton *button);
void sfButton_action(sfWindowState *window, sfButton *button);
void sfRenderWindow_drawButton(sfRenderWindow **window, sfButton *button);
void sfButton_setBox(sfButton *button, char *buttonText, int xPos, int yPos);
void sfButton_setLinedBox(sfButton *button, sfSprite *fromSprite,
													char *buttonText, int xPos, int yPos);
void sfText_setMiddlePos(sfButton *zone, int xPos, int yPos);
void sfRenderWindow_drawButtonLined(sfRenderWindow **window, sfButton *button);
void sfRenderWindow_drawButtonLined(sfRenderWindow **window, sfButton *button);
void sfButtonLined_destroy(sfButton *button);
void sfButton_shutOffOther(sfButton *button, int this, int size);
void sfText_setStringCenter(sfButton *button, char *buttonText, int xPos, int yPos);
void sfButton_replaceText(sfButton *button, char *buttonText);

// sfml.c prototypes
void openWindow(struct tfs *fs);

// sf_Inodes prototypes
int get_block_size(struct tfs *fs, int blockID);
int get_block(char *block, struct tfs *fs, int blockID, int size);
void sfInode_create(sfInode *inode, struct tfs *fs, sfWindowState *window);
void sfInode_action(sfWindowState *window, sfInode *sfInodes);
void sfRenderWindow_drawInodes(sfRenderWindow **window, sfInode *sfInodes);
void sfInode_destroy(sfInode *sfInodes);
void sfInode_Event(sfInode *sfInodes, struct tfs *fs);

#endif /* SFML_H_ */
