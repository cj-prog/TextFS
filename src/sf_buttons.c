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
 * Create default button
 * @button			-	sfButton structure
 * @buttonText 	- Text printed on the button
 * @width				- buttonwidth
 * @height			- buttonheight
 * */
void sfButton_create(sfButton *button, char *buttonText, int width, int height)
{
	button->text = sfText_create();
	button->sprite = sfSprite_create();
	button->texture = sfTexture_createFromFile(BUTTONTEXTURE, NULL);
	button->isClicked = 0;
	button->isDebounced = 0;
	button->font = sfFont_createFromFile(TEXTFONT);
	button->intRect.left = 0;
	button->intRect.top = sfTexture_getSize(button->texture).y * 0.5;
	button->intRect.height = height;
	button->intRect.width = width;

	sfSprite_setTextureRect(button->sprite, button->intRect);
	sfSprite_setTexture(button->sprite, button->texture, sfFalse);
	sfText_setFont(button->text, button->font);
	sfText_setCharacterSize(button->text, CHARSIZE);
	sfText_setColor(button->text, sfBlue);

	sfButton_setBox(button, buttonText, 0, 0);
}

/*
 * Destroy default button
 * @button			-	sfButton structure
 * */
void sfButton_destroy(sfButton *button)
{
	sfSprite_destroy(button->sprite);
	sfTexture_destroy(button->texture);
	sfText_destroy(button->text);
	sfFont_destroy(button->font);
}

/*
 * Destroy lined button
 * @button			-	sfButton structure
 * */
void sfButtonLined_destroy(sfButton *button)
{
	sfSprite_destroy(button->sprite);
	sfTexture_destroy(button->texture);
	sfText_destroy(button->text);
	sfFont_destroy(button->font);
	sfVertexArray_destroy(button->line);
}

/*
 * Mouse hover an click for button
 * @window	- window structure
 * @button	. button structure
 * */
void sfButton_action(sfWindowState *window, sfButton *button)
{
	// Check if the mouse in the button area
			// Left border
	if ((sfMouse_getPosition((sfWindow *) window->this).x
				>= (sfSprite_getPosition(button->sprite).x)
				 + window->resize_offset.x
				 - window->viewPosition.x + SCREENWIDTH * 0.5)
			&&
			// Right border
			(sfMouse_getPosition((sfWindow *) window->this).x
				<= sfSprite_getPosition(button->sprite).x
				 + sfSprite_getGlobalBounds(button->sprite).width
				 + window->resize_offset.x
				 - window->viewPosition.x + SCREENWIDTH * 0.5)
			&&
			// Lower border
			(sfMouse_getPosition((sfWindow *) window->this).y
				>= sfSprite_getPosition(button->sprite).y
				 - window->resize_offset.y
				 - window->viewPosition.y + SCREENHEIGHT * 0.5 )
			&&
			// Upper border
			(sfMouse_getPosition((sfWindow *) window->this).y
				<= sfSprite_getPosition(button->sprite).y
				 + sfSprite_getGlobalBounds(button->sprite).height
				 - window->resize_offset.y
				 - window->viewPosition.y + SCREENHEIGHT * 0.5))
	{
		// Change button color by setting the texture to the upper half
		button->intRect.top = sfTexture_getSize(button->texture).y * 0.5;
		sfSprite_setTextureRect(button->sprite, button->intRect);

		if (sfMouse_isButtonPressed(sfMouseLeft))
		{
			button->isDebounced = 1;
		}
	}
	else if (!button->isClicked)
	{
		button->intRect.top = 0;
		sfSprite_setTextureRect(button->sprite, button->intRect);
	}
}

/*
 * Center button text
 * @button	- button structure
 * @xPos		- x position of the button
 * @yPos		- y position of the button
 * */
void sfText_setMiddlePos(sfButton *button, int xPos, int yPos)
{
	sfVector2f buttonTextPosition;

	buttonTextPosition.x = xPos	+ sfSprite_getGlobalBounds(button->sprite).width * 0.5
															- sfText_getGlobalBounds(button->text).width * 0.5;

	buttonTextPosition.y = yPos	+ sfSprite_getGlobalBounds(button->sprite).height * 0.5
															- sfText_getGlobalBounds(button->text).height * 0.8;

	sfText_setPosition(button->text, buttonTextPosition);
}

/*
 * Replace button text and center
 * @button			- button structure
 * @buttonText	- text to be set
 * */
void sfButton_replaceText(sfButton *button, char *buttonText)
{
	sfText_setString( button->text, buttonText);
	sfText_setMiddlePos(button, sfSprite_getPosition(button->sprite).x,
															sfSprite_getPosition(button->sprite).y);
}

/*
 * Set button text and center
 * @button			- button structure
 * @buttonText	- text to be set
 * */
void sfText_setStringCenter(sfButton *button, char *buttonText,  int xPos, int yPos)
{
	sfText_setString(button->text, buttonText);
	sfText_setMiddlePos(button, xPos, yPos);
}

/*
 * Set button and its text position
 * @button			- button structure
 * @buttonText	- text to be set
 * @xPos				- x position of the button
 * @yPos				- y position of the button
 * */
void sfButton_setBox(sfButton *button, char *buttonText, int xPos, int yPos)
{
	sfSprite_setPosition(button->sprite, sfVector2f_set(xPos, yPos));
	sfText_setStringCenter(button, buttonText, xPos, yPos);
}

/*
 * Set lined button and its text position
 * @button			- button structure
 * @fromSprite	- begin of line
 * @buttonText	- text to be set
 * @xPos				- x position of the button
 * @yPos				- y position of the button
 * */
void sfButton_setLinedBox(sfButton *button, sfSprite *fromSprite,
													char *buttonText, int xPos, int yPos)
{
	sfButton_setBox(button, buttonText, xPos, yPos);
	sfVertexArray_setLine(button->line, fromSprite, button->sprite);
}

/*
 * Render button
 * @window	- window structure
 * @button	- button structure
 * */
void sfRenderWindow_drawButton(sfRenderWindow **window, sfButton *button)
{
	sfRenderWindow_drawSprite(*window, button->sprite, NULL);
	sfRenderWindow_drawText(*window, button->text, NULL);
}

/*
 * Render lined button
 * @button			- button structure
 * @fromSprite	- begin of line
 * @buttonText	- text to be set
 * @xPos				- x position of the button
 * @yPos				- y position of the button
 * */
void sfRenderWindow_drawButtonLined(sfRenderWindow **window, sfButton *button)
{
	sfRenderWindow_drawButton(window, button);
	sfRenderWindow_drawVertexArray(*window, button->line, NULL);
}
