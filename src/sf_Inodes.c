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
 * Get block size
 * @fs			-	file system structure
 * @blockID	- block id to read from
 * @return	- number of character
 * */
int get_block_size(struct tfs *fs, int blockID)
{
	if (blockID && blockID != -1 && blockID <= fs->sb->fs_sizeInBlocks )
	{
		char *ptr, *ptr2;

		ptr = goto_Block(fs->virtualFS,	blockID);
		ptr2 = strstr(ptr, "\n\n");

		return ptr2 - ptr;
	}
	else
		return FALSE;
}

/*
 * find block and copy
 * @block		-	captures data
 * @fs  		-	file system structure
 * @blockID	- block-number to find
 * @size		-	size of data to be copied
 * */
int get_block(char *block, struct tfs *fs, int blockID, int size)
{
	if (size && blockID > 2)
	{
		char *ptr;

		ptr = goto_Block(fs->virtualFS,	blockID);

		int i;
		for (i = 0; i < size; i++)
			block[i] = ptr[i];

		block[i] = '\0';
		return TRUE;
	}
	else
		strcpy(block, "Zone unused\0");
	return FALSE;
}

/**************************************************************************************************
 * create sfInodes
 *************************************************************************************************/

/*
 * Create inode content to be screened
 * @sfInodes	-	sfInode structure
 * @fs  			-	file system structure
 * @window		- window structure
 * @size			-	size of data to be copied
 * */
void sfInode_create(sfInode *sfInodes, struct tfs *fs, sfWindowState *window)
{
	int active;
	char buttonText[MAXBUTTONCHARS];

	sfInodes->activeInode = 0;
	sfInodes->activeZone = 0;
	sfInodes->activeExpand1 = 0;
	sfInodes->activeExpand2 = 0;
	sfInodes->activeExpand3 = 0;
	sfInodes->activeIndirZone = 0;

	sfInodes->font = sfFont_createFromFile(TEXTFONT);
	sfInodes->numberOfInodes = fs->sb->nInodes;

	sfText_mycreate(&sfInodes->info, "unresolved", &sfInodes->font,
									INFOCHARSIZE,
									X0(window),
									Y0(window));

	sfText_mycreate(&sfInodes->indirInfo, "unresolved", &sfInodes->font,
									INFOCHARSIZE,
									sfText_getPosition(sfInodes->info).x,
									sfText_getPosition(sfInodes->info).y + INFOLINES * CHARSIZE);


	sfText_mycreate(&sfInodes->doubleIndirInfo, "unresolved",
									&sfInodes->font,
									INFOCHARSIZE,
									sfText_getPosition(sfInodes->info).x,
									sfText_getPosition(sfInodes->info).y + INDIRINFOLINES * CHARSIZE);

	sfText_mycreate(&sfInodes->block,
									"unresolved", &sfInodes->font, CHARSIZE, 0, 0);

	sfInodes->blockLine = sfVertexArray_createLine();

	for (active = 0; active < fs->sb->nInodes; active++)
	{
		sprintf(buttonText, "inode %d", active + 1);

		sfButton_create(&sfInodes->inode[active], buttonText, BUTTONWIDTH,
										BUTTONHEIGHT);
	}

	for (active = 0; active < NR_OF_ZONES; active++)
	{
		if (active < INDIRZONE)
		{
			sprintf(buttonText, "zone %d", active);
		}
		else if (active == INDIRZONE)
		{
			sprintf(buttonText, "indirZ");
		}
		else
		{
			sprintf(buttonText, "2indirZ");
		}

		sfButton_create(&sfInodes->zone[active],
										buttonText,
										BUTTONWIDTH,
										BUTTONHEIGHT);

		sfInodes->zone[active].line = sfVertexArray_createLine();
	}

	for (active = 0; active < NR_OF_EXPANDS; active++)
	{
		sprintf(buttonText, "%d..%d", active * NR_OF_EXPANDS,
						active * NR_OF_EXPANDS + NR_OF_EXPANDS - 1);

		// expand1
		sfButton_create(&sfInodes->expand1[active],
										buttonText,
										BUTTONWIDTH,
										BUTTONHEIGHT);

		sfInodes->expand1[active].line = sfVertexArray_createLine();

		// expand3
		sfButton_create((sfButton *) &sfInodes->expand3[active],
										buttonText,
										BUTTONWIDTH,
										BUTTONHEIGHT);

		sfInodes->expand3[active].line = sfVertexArray_createLine();

		// expand2
		sprintf(buttonText, "indir %d", active);

		sfButton_create((sfButton *) &sfInodes->expand2[active],
										buttonText,
										BUTTONWIDTH,
										BUTTONHEIGHT);

		sfInodes->expand2[active].line = sfVertexArray_createLine();

		// indirect zones
		sprintf(buttonText, "iZone %d", active);

		sfButton_create((sfButton *) &sfInodes->indirZone[active],
										buttonText,
										BUTTONWIDTH,
										BUTTONHEIGHT);

		sfInodes->indirZone[active].line = sfVertexArray_createLine();
	}
}

/*
 * DEstroy all inode content
 * @sfInodes	-	sfInode structure
 * */
void sfInode_destroy(sfInode *sfInodes)
{
	int z;

	sfVertexArray_destroy(sfInodes->blockLine);
	sfText_destroy(sfInodes->block);
	sfText_destroy(sfInodes->info);
	sfText_destroy(sfInodes->indirInfo);
	sfText_destroy(sfInodes->doubleIndirInfo);
	sfFont_destroy(sfInodes->font);

	for (z = 0; z < sfInodes->numberOfInodes; z++)
	{
			sfButton_destroy((sfButton *) &sfInodes->inode[z]);
	}
	for (z = 0; z < NR_OF_ZONES; z++)
	{
			sfButtonLined_destroy((sfButton *) &sfInodes->zone[z]);
	}
	for (z = 0; z < NR_OF_EXPANDS; z++)
	{
		sfButtonLined_destroy((sfButton *) &sfInodes->expand1[z]);
		sfButtonLined_destroy((sfButton *) &sfInodes->expand2[z]);
		sfButtonLined_destroy((sfButton *) &sfInodes->expand3[z]);
		sfButtonLined_destroy((sfButton *) &sfInodes->indirZone[z]);
	}
}

/**************************************************************************************************
 * sfInode-action, check mouse-reaction and store state
 *************************************************************************************************/

/*
 * Handle clicked inode button
 * @window		-	window structure
 * @sfInodes 	-	sfInode structure
 * */
void sfInode_action(sfWindowState *window, sfInode *sfInodes)
{
	int i, z, iz, e2, e3;

	for (i = 0; i < sfInodes->numberOfInodes; i++)
	{
		sfButton_action(window, (sfButton *) &sfInodes->inode[i]);

		if (sfInodes->inode[i].isClicked)
		{
			for (z = 0; z < NR_OF_ZONES; z++)
			{
				sfButton_action(window, (sfButton *) &sfInodes->zone[z]);
			}
			if (sfInodes->zone[INDIRZONE].isClicked || sfInodes->zone[DOUBLEINDIRZONE].isClicked)
			{
				for (z = 0; z < NR_OF_EXPANDS; z++)
				{
					sfButton_action(window, (sfButton *) &sfInodes->expand1[z]);

					if (sfInodes->expand1[z].isClicked)
					{
						if (sfInodes->activeZone == INDIRZONE)
						{
							for (iz = 0; iz < NR_OF_EXPANDS; iz++)
							{
								sfButton_action(window, (sfButton *) &sfInodes->indirZone[iz]);
							}
						}
						else if (sfInodes->activeZone == DOUBLEINDIRZONE)
						{
							for (e2 = 0; e2 < NR_OF_EXPANDS; e2++)
							{
								sfButton_action(window, (sfButton *) &sfInodes->expand2[e2]);

								if (sfInodes->expand2[e2].isClicked)
								{
									for (e3 = 0; e3 < NR_OF_EXPANDS; e3++)
									{
										sfButton_action(window,	(sfButton *) &sfInodes->expand3[e3]);

										if (sfInodes->expand3[e3].isClicked)
										{
											for (iz = 0; iz < NR_OF_EXPANDS; iz++)
											{
												sfButton_action(window,	(sfButton *) &sfInodes->indirZone[iz]);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

/**************************************************************************************************
 * sfInode-event, handle event if button is clicked
 *************************************************************************************************/

/*
 * debounce indirect Zones
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * @this			- active indirZone
 * */
void indirZone_debounce(sfInode *sfInodes, struct tfs *fs, int this)
{
	int blockID;

	if (sfInodes->indirZone[this].isDebounced)
	{
		sfInodes->indirZone[sfInodes->activeIndirZone].isClicked = FALSE;
		sfInodes->activeIndirZone = this;
		TOGGLE_BUTTON(sfInodes->indirZone[this]);

		if (sfInodes->activeZone == INDIRZONE)
		{
			blockID = 	sfInodes->activeZone
								+ sfInodes->activeExpand1 * NR_OF_EXPANDS
								+ sfInodes->activeIndirZone;
		}
		else if(sfInodes->activeZone == DOUBLEINDIRZONE)
		{
			blockID = 	 sfInodes->activeZone
								+ (ADRESSES_PER_BLOCK - 1) 				// substract indirect zone
								+  sfInodes->activeExpand1 * NR_OF_EXPANDS * ADRESSES_PER_BLOCK
								+  sfInodes->activeExpand2 * ADRESSES_PER_BLOCK
								+  sfInodes->activeExpand3 * NR_OF_EXPANDS
								+  sfInodes->activeIndirZone;
		}

		sfText_setBlockString(sfInodes->block, fs,
													get_blockID_from_inode(fs, INODE(fs, sfInodes->activeInode + 1),
												                         blockID));

		sfText_setPosition( sfInodes->block,
												sfVector2f_set( sfSprite_getPosition(sfInodes->indirZone[0].sprite).x
																				+ XSPACE,
																				sfSprite_getPosition(sfInodes->indirZone[0].sprite).y));

		sfVertexArray_setBlockLine( sfInodes->blockLine,
																sfInodes->indirZone[this].sprite,
																sfInodes->indirZone[0].sprite);
	}
}

/*
 * debounce second expand line
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * @this			- active expand2
 * */
void expand3_debounce(sfInode *sfInodes, struct tfs *fs, int this)
{
	int e;

	if (sfInodes->expand3[this].isDebounced)
	{
		sfInodes->expand3[sfInodes->activeExpand3].isClicked = FALSE;
		sfInodes->indirZone[sfInodes->activeIndirZone].isClicked = FALSE;
		sfInodes->activeExpand3 = this;
		TOGGLE_BUTTON(sfInodes->expand3[this]);

		for (e = 0; e < NR_OF_EXPANDS; e++)
		{
			// move indirZone position
			sfButton_setLinedBox((sfButton *) &sfInodes->indirZone[e],
														sfInodes->expand3[this].sprite,
														(char *) sfText_getString(sfInodes->indirZone[e].text),
														(int) (sfSprite_getPosition(sfInodes->expand3[0].sprite).x + XSPACE),
														(int) (sfSprite_getPosition(sfInodes->expand3[e].sprite).y));
		}
	}
}

/*
 * debounce second expand line
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * @this			- active expand2
 * */
void expand2_debounce(sfInode *sfInodes, struct tfs *fs, int this)
{
	int e;
	int zone;
	int i0 = 2;

	if (sfInodes->expand2[this].isDebounced)
	{
		sfInodes->expand2[sfInodes->activeExpand2].isClicked = FALSE;
		sfInodes->indirZone[sfInodes->activeIndirZone].isClicked = FALSE;
		sfInodes->activeExpand2 = this;
		TOGGLE_BUTTON(sfInodes->expand2[this]);

		// Only first indirect block contains two holes
		if (sfInodes->activeExpand2 != 0 || sfInodes->activeExpand1 != 0)
		{
			i0 = 1;
		}

		for (e = 0; e < NR_OF_EXPANDS; e++)
		{
			// move indirZone position
			sfButton_setLinedBox((sfButton *) &sfInodes->expand3[e],
														sfInodes->expand2[this].sprite,
														(char *) sfText_getString(sfInodes->expand3[e].text),
														(int) (sfSprite_getPosition(sfInodes->expand2[0].sprite).x + XSPACE),
														(int) (sfSprite_getPosition(sfInodes->expand2[e].sprite).y));

			if(INODE(fs, sfInodes->activeInode + 1)->doubleIndirZone)
			{
				zone = 		 sfInodes->activeZone
								+ (ADRESSES_PER_BLOCK - 1) 												// substract indirect zone
								+  sfInodes->activeExpand1 * NR_OF_EXPANDS * ADRESSES_PER_BLOCK
								+  sfInodes->activeExpand2 * ADRESSES_PER_BLOCK;

				sfText_setBlockString(sfInodes->doubleIndirInfo, fs, i0
															+ get_blockID_from_inode(
																	fs, INODE(fs, sfInodes->activeInode + 1), zone));
			}else
			{
				sfText_setString(sfInodes->doubleIndirInfo, "unused");
			}
		}
	}
}

/*
 * debounce first expand line
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * @this			- active expand1
 * */
void expand1_debounce(sfInode *sfInodes, struct tfs *fs, int this)
{
	int e;
	char buttonText[MAXBUTTONCHARS];

	if (sfInodes->expand1[this].isDebounced)
	{
		sfInodes->indirZone[sfInodes->activeIndirZone].isClicked = FALSE;
		sfInodes->expand1[sfInodes->activeExpand1].isClicked = FALSE;
		sfInodes->expand2[sfInodes->activeExpand2].isClicked = FALSE;
		sfInodes->activeExpand1 = this;
		TOGGLE_BUTTON(sfInodes->expand1[this]);

		if (sfInodes->zone[INDIRZONE].isClicked)
		{
			for (e = 0; e < NR_OF_EXPANDS; e++)
			{
				// move indirZone position
				sfButton_setLinedBox( (sfButton *) &sfInodes->indirZone[e],
														  sfInodes->expand1[this].sprite,
															(char *) sfText_getString(sfInodes->indirZone[e].text),
															(int) (sfSprite_getPosition(sfInodes->expand1[0].sprite).x + XSPACE),
															(int) (sfSprite_getPosition(sfInodes->expand1[e].sprite).y));
			}
		}
		else if (sfInodes->zone[DOUBLEINDIRZONE].isClicked)
		{
			for (e = 0; e < NR_OF_EXPANDS; e++)
			{
				sprintf(buttonText, "%d", sfInodes->activeExpand1 * 16 + e);

				sfButton_setLinedBox(&sfInodes->expand2[e],
														sfInodes->expand1[this].sprite,
														buttonText,
														(int) (INODEPOS(sfInodes).x + 3 * XSPACE),
														(int) (INODEPOS(sfInodes).y	+ YSPACE * e * 0.6));
			}
		}
	}
}

/*
 * debounce inodes
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * @this			- active inode
 * */
void zone_debounce(sfInode *sfInodes, struct tfs *fs, int this)
{
	int e;

	if (sfInodes->zone[this].isDebounced)
	{
		sfInodes->zone[sfInodes->activeZone].isClicked 						= FALSE;
		sfInodes->expand1[sfInodes->activeExpand1].isClicked 			= FALSE;
		sfInodes->expand2[sfInodes->activeExpand2].isClicked			= FALSE;
		sfInodes->expand3[sfInodes->activeExpand3].isClicked 			= FALSE;
		sfInodes->indirZone[sfInodes->activeIndirZone].isClicked  = FALSE;
		sfInodes->activeZone																		  = this;

		TOGGLE_BUTTON(sfInodes->zone[this]);

		if (this < INDIRZONE)
		{
			sfText_setBlockString(sfInodes->block, fs,
														INODE(fs, sfInodes->activeInode + 1)->zones[sfInodes->activeZone]);

			sfText_setPosition( sfInodes->block,
													sfVector2f_set(
															sfSprite_getPosition(sfInodes->zone[0].sprite).x + XSPACE,
															sfSprite_getPosition(sfInodes->zone[0].sprite).y));

			sfVertexArray_setBlockLine( sfInodes->blockLine,
																	sfInodes->zone[this].sprite,
																	sfInodes->zone[0].sprite);
		}
		else if (this == INDIRZONE)
		{
			if(INODE(fs, sfInodes->activeInode + 1)->indirZone)
			{

				sfText_setBlockString(sfInodes->indirInfo, fs,
															INODE(fs, sfInodes->activeInode + 1)->indirZone);
			}else
			{
				sfText_setString(sfInodes->indirInfo, "unused");
			}

			for (e = 0; e < NR_OF_EXPANDS; e++)
			{
				sfButton_setLinedBox(&sfInodes->expand1[e],
														 sfInodes->zone[sfInodes->activeZone].sprite,
														 (char *) sfText_getString(sfInodes->expand1[e].text),
														 (int) (INODEPOS(sfInodes).x + 2 * XSPACE),
														 (int) (INODEPOS(sfInodes).y
														  + YSPACE * e * 0.6));
			}
		}
		else if (this == DOUBLEINDIRZONE)
		{
			if(INODE(fs, sfInodes->activeInode + 1)->doubleIndirZone)
			{
				sfText_setBlockString(sfInodes->indirInfo, fs,
															INODE(fs, sfInodes->activeInode + 1)->doubleIndirZone);
			}else
			{
				sfText_setString(sfInodes->indirInfo, "unused");
				sfText_setString(sfInodes->doubleIndirInfo, "unused");
			}

			for (e = 0; e < NR_OF_EXPANDS; e++)
			{
				sfButton_setLinedBox(&sfInodes->expand1[e],
														 sfInodes->zone[sfInodes->activeZone].sprite,
														 (char *) sfText_getString(sfInodes->expand1[e].text),
														 (int) (INODEPOS(sfInodes).x + 2 * XSPACE),
														 (int) (INODEPOS(sfInodes).y
														  + YSPACE * e * 0.6));
			}
		}
	}
}

/*
 * debounce inodes
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * @this			- active inode
 * */
void inodeDebounce(sfInode *sfInodes, struct tfs *fs, int this)
{
	int i;

	if (sfInodes->inode[this].isDebounced)
	{
		sfInodes->inode[sfInodes->activeInode].isClicked = FALSE;
		sfInodes->zone[sfInodes->activeZone].isClicked = 0;
		sfInodes->activeInode = this;
		TOGGLE_BUTTON(sfInodes->inode[sfInodes->activeInode]);

		for (i = 0; i < NR_OF_ZONES; i++)
		{
			sfButton_setLinedBox(&sfInodes->zone[i],
													 sfInodes->inode[sfInodes->activeInode].sprite,
													 (char *) sfText_getString(sfInodes->zone[i].text),
													 (int) (INODEPOS(sfInodes).x + XSPACE),
													 (int) (INODEPOS(sfInodes).y + YSPACE * 0.5	+ YSPACE * i));
		}

		sfText_setBlockString(sfInodes->info, fs, SB_POSITION
																							+ ((fs)->sb->zmap_sizeInBlocks)
																							+ (fs->sb->imap_sizeInBlocks)
																							+ sfInodes->activeInode + 1);
	}
}

/*
 * Handle inode event on user click
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * */
void sfInode_Event(sfInode *sfInodes, struct tfs *fs)
{
	int i, z, e, iz, e2, e3;

	for (i = 0; i < fs->sb->nInodes; i++)
	{
		inodeDebounce(sfInodes, fs, i);
	}
	if (sfInodes->inode[sfInodes->activeInode].isClicked)
	{
		for (z = 0; z < NR_OF_ZONES; z++)
		{
			zone_debounce(sfInodes, fs, z);
		}
		if (sfInodes->zone[INDIRZONE].isClicked	|| sfInodes->zone[DOUBLEINDIRZONE].isClicked)
		{
			for (e = 0; e < NR_OF_EXPANDS; e++)
			{
				expand1_debounce(sfInodes, fs, e);
			}
			if (sfInodes->expand1[sfInodes->activeExpand1].isClicked)
			{
				if (sfInodes->activeZone == INDIRZONE)
				{
					for (iz = 0; iz < NR_OF_EXPANDS; iz++)
					{
						indirZone_debounce(sfInodes, fs, iz);
					}
				}
				else if (sfInodes->activeZone == DOUBLEINDIRZONE)
				{
					for (e2 = 0; e2 < NR_OF_EXPANDS; e2++)
					{
						expand2_debounce(sfInodes, fs, e2);
					}
					if (sfInodes->expand2[sfInodes->activeExpand2].isClicked)
					{
						for (e3 = 0; e3 < NR_OF_EXPANDS; e3++)
						{
							expand3_debounce(sfInodes, fs, e3);
						}
						if (sfInodes->expand3[sfInodes->activeExpand3].isClicked)
						{
							for (iz = 0; iz < NR_OF_EXPANDS; iz++)
							{
								indirZone_debounce(sfInodes, fs, iz);
							}
						}
					}
				}
			}
		}
	}
}

/**************************************************************************************************
 * draw sfInodes
 *************************************************************************************************/

/*
 * Handle inode event on user click
 * @sfInodes 	-	sfInode structure
 * @fs				-	file system structure
 * */
void sfRenderWindow_drawBlock(sfRenderWindow **window, sfInode *sfInodes)
{
	sfRenderWindow_drawText(*window, sfInodes->block, NULL);
	sfRenderWindow_drawVertexArray(*window, sfInodes->blockLine, NULL);
}

/*
 * render inode content
 * @window 		-	window structure
 * @sfInodes	-	sfInodes structure
 * */
void sfRenderWindow_drawInodes(sfRenderWindow **window, sfInode *sfInodes)
{
	int activeInode, z, iz, e, e2, e3;

	for (activeInode = 0; activeInode < sfInodes->numberOfInodes; activeInode++)
	{
		sfRenderWindow_drawButton(window, &sfInodes->inode[activeInode]);

		if (sfInodes->inode[activeInode].isClicked)
		{
			sfRenderWindow_drawText(*window, sfInodes->info, NULL);

			for (z = 0; z < NR_OF_ZONES; z++)
			{
				sfRenderWindow_drawButtonLined(window, &sfInodes->zone[z]);
			}

			if (sfInodes->zone[sfInodes->activeZone].isClicked)
			{
				if (sfInodes->activeZone < 7)
				{
					sfRenderWindow_drawText(*window, sfInodes->block, NULL);
					sfRenderWindow_drawVertexArray(*window, sfInodes->blockLine, NULL);
				}
				else
				{
					sfRenderWindow_drawText(*window, sfInodes->indirInfo, NULL);

					for (e = 0; e < NR_OF_EXPANDS; e++)
					{
						sfRenderWindow_drawButtonLined(window, &sfInodes->expand1[e]);
					}

					if (sfInodes->zone[INDIRZONE].isClicked
							&& sfInodes->expand1[sfInodes->activeExpand1].isClicked)
					{
						for (iz = 0; iz < NR_OF_EXPANDS; iz++)
						{
							sfRenderWindow_drawButtonLined(window, &sfInodes->indirZone[iz]);
						}

						if (sfInodes->indirZone[sfInodes->activeIndirZone].isClicked)
						{
							sfRenderWindow_drawText(*window, sfInodes->block, NULL);
							sfRenderWindow_drawVertexArray(*window, sfInodes->blockLine,
							NULL);
						}
					}
					else if (sfInodes->zone[DOUBLEINDIRZONE].isClicked
							&& sfInodes->expand1[sfInodes->activeExpand1].isClicked)
					{
						for (e2 = 0; e2 < NR_OF_EXPANDS; e2++)
						{
							sfRenderWindow_drawButtonLined(window, &sfInodes->expand2[e2]);
						}

						if (sfInodes->expand2[sfInodes->activeExpand2].isClicked)
						{
							sfRenderWindow_drawText(*window, sfInodes->doubleIndirInfo, NULL);

							for (e3 = 0; e3 < NR_OF_EXPANDS; e3++)
							{
								sfRenderWindow_drawButtonLined(window, &sfInodes->expand3[e3]);
							}
							if (sfInodes->expand3[sfInodes->activeExpand3].isClicked)
							{
								for (iz = 0; iz < NR_OF_EXPANDS; iz++)
								{
									sfRenderWindow_drawButtonLined(window, &sfInodes->indirZone[iz]);
								}
								if (sfInodes->indirZone[sfInodes->activeIndirZone].isClicked)
								{

									sfRenderWindow_drawText(*window, sfInodes->block, NULL);
									sfRenderWindow_drawVertexArray(*window, sfInodes->blockLine,
									NULL);
								}
							}
						}
					}
				}
			}
		}
	}
}
