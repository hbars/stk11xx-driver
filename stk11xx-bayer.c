/** 
 * @file stk11xx-bayer.c
 * @author Martin ROOS
 * @date 2006-01-14
 * @version v2.2.x
 *
 * @brief Driver for Syntek USB video camera
 *
 * @note Copyright (C) Nicolas VIVIEN
 *
 * @par Licences
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * @par SubVersion
 *   $Date: $
 *   $Revision: $
 *   $Author: $
 *   $HeadURL: $
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/kref.h>

#include <linux/usb.h>
#include <media/v4l2-common.h>

#include "stk11xx.h"


#define MAX(a,b)	((a)>(b)?(a):(b))
#define MIN(a,b)	((a)<(b)?(a):(b))
#define CLIP(a,low,high) MAX((low),MIN((high),(a)))


void stk11xx_b2rgb24(uint8_t *, uint8_t *,
		struct stk11xx_coord *, struct stk11xx_coord *,
		const int, const int, const int);
void stk11xx_b2rgb32(uint8_t *, uint8_t *, 
		struct stk11xx_coord *, struct stk11xx_coord *, 
		const int, const int, const int);
void stk11xx_b2bgr24(uint8_t *, uint8_t *,
		struct stk11xx_coord *, struct stk11xx_coord *,
		const int, const int, const int);
void stk11xx_b2bgr32(uint8_t *, uint8_t *,
		struct stk11xx_coord *, struct stk11xx_coord *,
		const int, const int, const int);

void stk11xx_b2uyvy(uint8_t *, uint8_t *,
		struct stk11xx_coord *, struct stk11xx_coord *,
		const int, const int, const int);
void stk11xx_b2yuyv(uint8_t *, uint8_t *,
		struct stk11xx_coord *, struct stk11xx_coord *,
		const int, const int, const int);


void stk11xx_correct_brightness(uint8_t *, const int, const int,
		const int, int, int);


static signed short stk11xx_yuv_interp[256][8] = {
	{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,1,0,0,0,1,0,0},{0,1,0,0,0,1,-1,0},
	{1,2,0,0,-1,2,-1,0},{1,2,0,0,-1,2,-2,0},{1,3,0,-1,-1,3,-2,0},{2,3,0,-1,-2,3,-2,0},
	{2,4,0,-1,-2,4,-3,0},{2,5,1,-1,-2,4,-3,0},{2,5,1,-1,-3,5,-4,0},{3,6,1,-1,-3,5,-4,0},
	{3,6,1,-2,-3,6,-5,0},{3,7,1,-2,-4,6,-5,-1},{4,7,1,-2,-4,7,-5,-1},{4,8,1,-2,-4,7,-6,-1},
	{4,9,1,-2,-5,8,-6,-1},{5,9,1,-2,-5,8,-7,-1},{5,10,2,-3,-5,9,-7,-1},{5,10,2,-3,-6,9,-7,-1},
	{5,11,2,-3,-6,10,-8,-1},{6,11,2,-3,-6,10,-8,-1},{6,12,2,-3,-7,11,-9,-1},{6,13,2,-3,-7,11,-9,-1},
	{7,13,2,-4,-7,12,-10,-1},{7,14,2,-4,-8,12,-10,-2},{7,14,2,-4,-8,13,-10,-2},{8,15,3,-4,-8,13,-11,-2},
	{8,15,3,-4,-9,14,-11,-2},{8,16,3,-4,-9,14,-12,-2},{8,17,3,-5,-9,15,-12,-2},{9,17,3,-5,-10,15,-12,-2},
	{9,18,3,-5,-10,16,-13,-2},{9,18,3,-5,-10,16,-13,-2},{10,19,3,-5,-11,17,-14,-2},{10,19,3,-5,-11,17,-14,-2},
	{10,20,4,-6,-11,18,-15,-2},{11,20,4,-6,-12,18,-15,-3},{11,21,4,-6,-12,19,-15,-3},{11,22,4,-6,-12,19,-16,-3},
	{11,22,4,-6,-13,20,-16,-3},{12,23,4,-6,-13,20,-17,-3},{12,23,4,-7,-13,21,-17,-3},{12,24,4,-7,-14,21,-18,-3},
	{13,24,5,-7,-14,22,-18,-3},{13,25,5,-7,-14,22,-18,-3},{13,26,5,-7,-15,23,-19,-3},{14,26,5,-7,-15,23,-19,-3},
	{14,27,5,-8,-15,24,-20,-3},{14,27,5,-8,-16,24,-20,-3},{14,28,5,-8,-16,25,-20,-4},{15,28,5,-8,-16,25,-21,-4},
	{15,29,5,-8,-17,26,-21,-4},{15,30,6,-8,-17,26,-22,-4},{16,30,6,-9,-17,27,-22,-4},{16,31,6,-9,-18,27,-23,-4},
	{16,31,6,-9,-18,28,-23,-4},{17,32,6,-9,-18,28,-23,-4},{17,32,6,-9,-19,29,-24,-4},{17,33,6,-9,-19,29,-24,-4},
	{17,34,6,-10,-19,30,-25,-4},{18,34,6,-10,-20,30,-25,-4},{18,35,7,-10,-20,31,-25,-5},{18,35,7,-10,-20,31,-26,-5},
	{19,36,7,-10,-21,32,-26,-5},{19,36,7,-10,-21,32,-27,-5},{19,37,7,-11,-21,33,-27,-5},{20,37,7,-11,-22,33,-28,-5},
	{20,38,7,-11,-22,34,-28,-5},{20,39,7,-11,-22,34,-28,-5},{20,39,7,-11,-23,35,-29,-5},{21,40,8,-11,-23,35,-29,-5},
	{21,40,8,-12,-23,36,-30,-5},{21,41,8,-12,-24,36,-30,-5},{22,41,8,-12,-24,37,-30,-6},{22,42,8,-12,-24,37,-31,-6},
	{22,43,8,-12,-25,38,-31,-6},{23,43,8,-12,-25,38,-32,-6},{23,44,8,-13,-25,39,-32,-6},{23,44,9,-13,-26,39,-33,-6},
	{23,45,9,-13,-26,40,-33,-6},{24,45,9,-13,-26,40,-33,-6},{24,46,9,-13,-27,41,-34,-6},{24,47,9,-14,-27,41,-34,-6},
	{25,47,9,-14,-27,42,-35,-6},{25,48,9,-14,-28,42,-35,-6},{25,48,9,-14,-28,43,-36,-6},{26,49,9,-14,-28,43,-36,-7},
	{26,49,10,-14,-29,44,-36,-7},{26,50,10,-15,-29,44,-37,-7},{26,51,10,-15,-29,45,-37,-7},{27,51,10,-15,-30,45,-38,-7},
	{27,52,10,-15,-30,46,-38,-7},{27,52,10,-15,-30,46,-38,-7},{28,53,10,-15,-31,47,-39,-7},{28,53,10,-16,-31,47,-39,-7},
	{28,54,10,-16,-31,48,-40,-7},{29,54,11,-16,-32,48,-40,-7},{29,55,11,-16,-32,49,-41,-7},{29,56,11,-16,-32,49,-41,-8},
	{29,56,11,-16,-33,50,-41,-8},{30,57,11,-17,-33,50,-42,-8},{30,57,11,-17,-33,51,-42,-8},{30,58,11,-17,-34,51,-43,-8},
	{31,58,11,-17,-34,52,-43,-8},{31,59,11,-17,-34,52,-43,-8},{31,60,12,-17,-35,53,-44,-8},{31,60,12,-18,-35,53,-44,-8},
	{32,61,12,-18,-35,54,-45,-8},{32,61,12,-18,-36,54,-45,-8},{32,62,12,-18,-36,55,-46,-8},{33,62,12,-18,-36,55,-46,-9},
	{33,63,12,-18,-37,56,-46,-9},{33,64,12,-19,-37,56,-47,-9},{34,64,12,-19,-37,57,-47,-9},{34,65,13,-19,-38,57,-48,-9},
	{34,65,13,-19,-38,58,-48,-9},{34,66,13,-19,-38,58,-48,-9},{35,66,13,-19,-39,59,-49,-9},{35,67,13,-20,-39,59,-49,-9},
	{35,68,13,-20,-39,60,-50,-9},{36,68,13,-20,-40,60,-50,-9},{36,69,13,-20,-40,61,-51,-9},{36,69,14,-20,-40,61,-51,-9},
	{37,70,14,-20,-41,62,-51,-10},{37,70,14,-21,-41,62,-52,-10},{37,71,14,-21,-41,63,-52,-10},{37,72,14,-21,-42,63,-53,-10},
	{38,72,14,-21,-42,64,-53,-10},{38,73,14,-21,-42,64,-54,-10},{38,73,14,-21,-43,65,-54,-10},{39,74,14,-22,-43,65,-54,-10},
	{39,74,15,-22,-43,66,-55,-10},{39,75,15,-22,-44,66,-55,-10},{40,75,15,-22,-44,67,-56,-10},{40,76,15,-22,-44,67,-56,-10},
	{40,77,15,-22,-45,68,-56,-11},{40,77,15,-23,-45,68,-57,-11},{41,78,15,-23,-45,69,-57,-11},{41,78,15,-23,-46,69,-58,-11},
	{41,79,15,-23,-46,70,-58,-11},{42,79,16,-23,-46,70,-59,-11},{42,80,16,-23,-47,71,-59,-11},{42,81,16,-24,-47,71,-59,-11},
	{43,81,16,-24,-47,72,-60,-11},{43,82,16,-24,-48,72,-60,-11},{43,82,16,-24,-48,73,-61,-11},{43,83,16,-24,-48,73,-61,-11},
	{44,83,16,-24,-49,74,-61,-12},{44,84,16,-25,-49,74,-62,-12},{44,85,17,-25,-49,75,-62,-12},{45,85,17,-25,-50,75,-63,-12},
	{45,86,17,-25,-50,76,-63,-12},{45,86,17,-25,-50,76,-64,-12},{46,87,17,-25,-51,77,-64,-12},{46,87,17,-26,-51,77,-64,-12},
	{46,88,17,-26,-51,78,-65,-12},{46,89,17,-26,-52,78,-65,-12},{47,89,18,-26,-52,79,-66,-12},{47,90,18,-26,-52,79,-66,-12},
	{47,90,18,-26,-53,80,-66,-13},{48,91,18,-27,-53,80,-67,-13},{48,91,18,-27,-53,81,-67,-13},{48,92,18,-27,-54,81,-68,-13},
	{49,92,18,-27,-54,82,-68,-13},{49,93,18,-27,-54,82,-69,-13},{49,94,18,-28,-54,83,-69,-13},{49,94,19,-28,-55,83,-69,-13},
	{50,95,19,-28,-55,84,-70,-13},{50,95,19,-28,-55,84,-70,-13},{50,96,19,-28,-56,85,-71,-13},{51,96,19,-28,-56,85,-71,-13},
	{51,97,19,-29,-56,86,-72,-13},{51,98,19,-29,-57,86,-72,-14},{52,98,19,-29,-57,87,-72,-14},{52,99,19,-29,-57,87,-73,-14},
	{52,99,20,-29,-58,88,-73,-14},{52,100,20,-29,-58,88,-74,-14},{53,100,20,-30,-58,89,-74,-14},{53,101,20,-30,-59,89,-74,-14},
	{53,102,20,-30,-59,90,-75,-14},{54,102,20,-30,-59,90,-75,-14},{54,103,20,-30,-60,91,-76,-14},{54,103,20,-30,-60,91,-76,-14},
	{55,104,20,-31,-60,92,-77,-14},{55,104,21,-31,-61,92,-77,-15},{55,105,21,-31,-61,93,-77,-15},{55,106,21,-31,-61,93,-78,-15},
	{56,106,21,-31,-62,94,-78,-15},{56,107,21,-31,-62,94,-79,-15},{56,107,21,-32,-62,95,-79,-15},{57,108,21,-32,-63,95,-79,-15},
	{57,108,21,-32,-63,96,-80,-15},{57,109,22,-32,-63,96,-80,-15},{58,109,22,-32,-64,97,-81,-15},{58,110,22,-32,-64,97,-81,-15},
	{58,111,22,-33,-64,98,-82,-15},{58,111,22,-33,-65,98,-82,-16},{59,112,22,-33,-65,99,-82,-16},{59,112,22,-33,-65,99,-83,-16},
	{59,113,22,-33,-66,100,-83,-16},{60,113,22,-33,-66,100,-84,-16},{60,114,23,-34,-66,101,-84,-16},{60,115,23,-34,-67,101,-84,-16},
	{60,115,23,-34,-67,102,-85,-16},{61,116,23,-34,-67,102,-85,-16},{61,116,23,-34,-68,103,-86,-16},{61,117,23,-34,-68,103,-86,-16},
	{62,117,23,-35,-68,104,-87,-16},{62,118,23,-35,-69,104,-87,-16},{62,119,23,-35,-69,105,-87,-17},{63,119,24,-35,-69,105,-88,-17},
	{63,120,24,-35,-70,106,-88,-17},{63,120,24,-35,-70,106,-89,-17},{63,121,24,-36,-70,107,-89,-17},{64,121,24,-36,-71,107,-90,-17},
	{64,122,24,-36,-71,108,-90,-17},{64,123,24,-36,-71,108,-90,-17},{65,123,24,-36,-72,109,-91,-17},{65,124,24,-36,-72,109,-91,-17},
	{65,124,25,-37,-72,110,-92,-17},{66,125,25,-37,-73,110,-92,-17},{66,125,25,-37,-73,111,-92,-18},{66,126,25,-37,-73,111,-93,-18},
	{66,127,25,-37,-74,112,-93,-18},{67,127,25,-37,-74,112,-94,-18},{67,128,25,-38,-74,113,-94,-18},{67,128,25,-38,-75,113,-95,-18},
	{68,129,25,-38,-75,114,-95,-18},{68,129,26,-38,-75,114,-95,-18},{68,130,26,-38,-76,115,-96,-18},{69,130,26,-38,-76,115,-96,-18},
	{69,131,26,-39,-76,116,-97,-18},{69,132,26,-39,-77,116,-97,-18},{69,132,26,-39,-77,117,-97,-19},{70,133,26,-39,-77,117,-98,-19},
	{70,133,26,-39,-78,118,-98,-19},{70,134,27,-39,-78,118,-99,-19},{71,134,27,-40,-78,119,-99,-19},{71,135,27,-40,-79,119,-100,-19},
	{71,136,27,-40,-79,120,-100,-19},{72,136,27,-40,-79,120,-100,-19},{72,137,27,-40,-80,121,-101,-19},{72,137,27,-40,-80,121,-101,-19},
	{72,138,27,-41,-80,122,-102,-19},{73,138,27,-41,-81,122,-102,-19},{73,139,28,-41,-81,123,-103,-19},{73,140,28,-41,-81,123,-103,-20},
	{74,140,28,-41,-82,124,-103,-20},{74,141,28,-42,-82,124,-104,-20},{74,141,28,-42,-82,125,-104,-20},{75,142,28,-42,-83,125,-105,-20},
	{75,142,28,-42,-83,126,-105,-20},{75,143,28,-42,-83,126,-105,-20},{75,144,28,-42,-84,127,-106,-20},{76,144,29,-43,-84,127,-106,-20}
};


/** 
 * @brief Decompress a frame
 *
 * This function permits to decompress a frame from the video stream.
 *
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 */
int stk11xx_decompress(struct usb_stk11xx *dev)
{
	int factor;

	void *data;
	void *image;
	struct stk11xx_frame_buf *framebuf;

	if (dev == NULL)
		return -EFAULT;

	framebuf = dev->read_frame;

	if (framebuf == NULL)
		return -EFAULT;

	image  = dev->image_data;
	image += dev->images[dev->fill_image].offset;

	data = framebuf->data;

	switch (dev->resolution) {
		case STK11XX_80x60:
			factor = 8;
			break;

		case STK11XX_128x96:
			factor = 5;
			break;

		case STK11XX_160x120:
			factor = 4;
			break;

		case STK11XX_213x160:
			factor = 3;
			break;

		case STK11XX_320x240:
			factor = 2;
			break;

		case STK11XX_640x480:
			factor = 1;
			break;

		case STK11XX_720x576:
		case STK11XX_800x600:
			factor = 2;
			break;

		case STK11XX_1024x768:
			factor = 2;
			break;

		case STK11XX_1280x1024:
			factor = 1;
			break;

		default:
			return -EFAULT;
	}


	switch (dev->vsettings.palette) {
		case STK11XX_PALETTE_RGB24:
			stk11xx_b2rgb24(data, image, &dev->image, &dev->view,
				dev->vsettings.hflip, dev->vsettings.vflip, factor);
			break;

		case STK11XX_PALETTE_RGB32:
			stk11xx_b2rgb32(data, image, &dev->image, &dev->view,
				dev->vsettings.hflip, dev->vsettings.vflip, factor);
			break;

		case STK11XX_PALETTE_BGR24:
			stk11xx_b2bgr24(data, image, &dev->image, &dev->view,
				dev->vsettings.hflip, dev->vsettings.vflip, factor);
			break;

		case STK11XX_PALETTE_BGR32:
			stk11xx_b2bgr32(data, image, &dev->image, &dev->view,
				dev->vsettings.hflip, dev->vsettings.vflip, factor);
			break;

		case STK11XX_PALETTE_UYVY:
			stk11xx_b2uyvy(data, image, &dev->image, &dev->view,
				dev->vsettings.hflip, dev->vsettings.vflip, factor);
			break;

		case STK11XX_PALETTE_YUYV:
			stk11xx_b2yuyv(data, image, &dev->image, &dev->view,
				dev->vsettings.hflip, dev->vsettings.vflip, factor);
			break;
	}

	stk11xx_correct_brightness(image, dev->view.x, dev->view.y,
		dev->vsettings.brightness, dev->vsettings.palette, dev->vsettings.depth);

	return 0;
}


/** 
 * @brief Correct the brightness of an image.
 *
 * This function permits to correct the brightness of an image.
 *
 * @param img Buffer to RGB/YUV data
 * @param width Width of frame
 * @param height Height of frame
 * @param brightness Brightness correction
 * @param depth Color depth
 *
 * @retval rgb Buffer to RGB/YUV data
 */
void stk11xx_correct_brightness(uint8_t *img, const int width, const int height, 
		const int brightness, int palette, int depth)
{
	int i;
	int x;


	switch (palette) {
		case STK11XX_PALETTE_RGB24:
		case STK11XX_PALETTE_BGR24:
		case STK11XX_PALETTE_RGB32:
		case STK11XX_PALETTE_BGR32:
			depth = (depth == 24) ? 3 : 4;

			if (brightness >= 32767) {
				x = (brightness - 32767) / 256;

				for (i = 0; i < (width * height * depth); i++) {
					if ((*(img + i) + (unsigned char) x) > 255)
						*(img + i) = 255;
					else
						*(img + i) += (unsigned char) x;
				}
			}
			else {
				x = (32767 - brightness) / 256;
		
				for (i = 0; i < (width * height * depth); i++) {
					if ((unsigned char) x > *(img + i))
						*(img + i) = 0;
					else
						*(img + i) -= (unsigned char) x;
				}
			}

			break;

		case STK11XX_PALETTE_UYVY:
			depth = 2;

			if (brightness >= 32767) {
				x = (brightness - 32767) / 256;

				for (i = 1; i < (width * height * depth); i=i+depth) {
					if ((*(img + i) + (unsigned char) x) > 255)
						*(img + i) = 255;
					else
						*(img + i) += (unsigned char) x;
				}
			}
			else {
				x = (32767 - brightness) / 256;
		
				for (i = 1; i < (width * height * depth); i=i+depth) {
					if ((unsigned char) x > *(img + i))
						*(img + i) = 0;
					else
						*(img + i) -= (unsigned char) x;
				}
			}

			break;

		case STK11XX_PALETTE_YUYV:
			depth = 2;

			if (brightness >= 32767) {
				x = (brightness - 32767) / 256;

				for (i = 0; i < (width * height * depth); i=i+depth) {
					if ((*(img + i) + (unsigned char) x) > 255)
						*(img + i) = 255;
					else
						*(img + i) += (unsigned char) x;
				}
			}
			else {
				x = (32767 - brightness) / 256;
		
				for (i = 0; i < (width * height * depth); i=i+depth) {
					if ((unsigned char) x > *(img + i))
						*(img + i) = 0;
					else
						*(img + i) -= (unsigned char) x;
				}
			}

			break;
	}
}


/** 
 * @brief This function permits to convert an image from bayer to RGB24
 *
 * @param bayer Buffer with the bayer data
 * @param image Size of image
 * @param view Size of view
 * @param hflip Horizontal flip
 * @param vflip Vertical flip
 * @param factor Factor of redimensioning
 *
 * @retval rgb Buffer with the RGB data
 */
void stk11xx_b2rgb24(uint8_t *bayer, uint8_t *rgb,
		struct stk11xx_coord *image,
		struct stk11xx_coord *view,
		const int hflip, const int vflip,
		const int factor) {
	uint8_t *b;

	int x, y; // Position in bayer image
	int i, j; // Position in rgb image

	int width = image->x;
	int height = image->y;

	int nwidth = width / factor;
	int nheight = height / factor;

	int offset;
	int startx, stepx;
	int starty, stepy;


	// Calculate the initial position (on Y axis)
	if (vflip) {
		starty = height - 2;
		stepy = -factor;
	}
	else {
		starty = 0;
		stepy = factor;
	}

	// Calculate the initial position (on X axis)
	if (hflip) {
		startx = width - 1;
		stepx = -factor;
		offset = width - 2;
	}
	else {
		startx = 0;
		stepx = factor;
		offset = 1;
	}


	// Skip the first line
	bayer += width;

	// To center vertically the image in the view
	rgb += ((view->y - nheight) / 2) * view->x * 3;

	// To center horizontally the image in the view
	rgb += ((view->x - nwidth) / 2) * 3;

	// Clean the first line
	memset(rgb, 0, nwidth * 3);
	rgb += nwidth * 3;


	// For each rgb line without the borders (first and last line)
	for (j=0, y=starty; j<nheight-2; j++, y=y+stepy) {
		// Go to the start of line
		b = bayer + y * width + offset;

		// Offset to center horizontally the image in the view
		rgb += (view->x - nwidth) * 3;

		if (y & 0x1) {
			// Skip the first pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;

			// GBGBGB : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*rgb++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					*rgb++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*rgb++ = *b;
				}
				else {
					*rgb++ = (*(b-width) + *(b+width)) >> 1;
					*rgb++ = *b;
					*rgb++ = (*(b-1) + *(b+1)) >> 1;
				}

				b += stepx;
			}

			// Skip the last pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
		}
		else {
			// Skip the first pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;

			// RGRGRG : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*rgb++ = (*(b-1) + *(b+1)) >> 1;
					*rgb++ = *b;
					*rgb++ = (*(b-width) + *(b+width)) >> 1;
				}
				else {
					*rgb++ = *b;
					*rgb++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*rgb++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
				}
	
				b += stepx;
			}

			// Skip the last pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
		}
	}

	// Clean the last line
	memset(rgb, 0, nwidth * 3);
}


/** 
 * @brief This function permits to convert an image from bayer to RGB32
 *
 * @param bayer Buffer with the bayer data
 * @param image Size of image
 * @param view Size of view
 * @param hflip Horizontal flip
 * @param vflip Vertical flip
 * @param factor Factor of redimensioning
 *
 * @retval rgb Buffer with the RGB data
 */
void stk11xx_b2rgb32(uint8_t *bayer, uint8_t *rgb,
		struct stk11xx_coord *image,
		struct stk11xx_coord *view,
		const int hflip, const int vflip,
		const int factor) {
	uint8_t *b;

	int x, y; // Position in bayer image
	int i, j; // Position in rgb image

	int width = image->x;
	int height = image->y;

	int nwidth = width / factor;
	int nheight = height / factor;

	int offset;
	int startx, stepx;
	int starty, stepy;


	// Calculate the initial position (on Y axis)
	if (vflip) {
		starty = height - 2;
		stepy = -factor;
	}
	else {
		starty = 0;
		stepy = factor;
	}

	// Calculate the initial position (on X axis)
	if (hflip) {
		startx = width - 1;
		stepx = -factor;
		offset = width - 2;
	}
	else {
		startx = 0;
		stepx = factor;
		offset = 1;
	}


	// Skip the first line
	bayer += width;

	// To center vertically the image in the view
	rgb += ((view->y - nheight) / 2) * view->x * 4;

	// To center horizontally the image in the view
	rgb += ((view->x - nwidth) / 2) * 4;

	// Clean the first line
	memset(rgb, 0, nwidth * 4);
	rgb += nwidth * 4;


	// For each rgb line without the borders (first and last line)
	for (j=0, y=starty; j<nheight-2; j++, y=y+stepy) {
		// Go to the start of line
		b = bayer + y * width + offset;

		// Offset to center horizontally the image in the view
		rgb += (view->x - nwidth) * 4;

		if (y & 0x1) {
			// Skip the first pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;

			// GBGBGB : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*rgb++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					*rgb++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*rgb++ = *b;
					*rgb++ = 0;
				}
				else {
					*rgb++ = (*(b-width) + *(b+width)) >> 1;
					*rgb++ = *b;
					*rgb++ = (*(b-1) + *(b+1)) >> 1;
					*rgb++ = 0;
				}

				b += stepx;
			}

			// Skip the last pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
		}
		else {
			// Skip the first pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;

			// RGRGRG : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*rgb++ = (*(b-1) + *(b+1)) >> 1;
					*rgb++ = *b;
					*rgb++ = (*(b-width) + *(b+width)) >> 1;
					*rgb++ = 0;
				}
				else {
					*rgb++ = *b;
					*rgb++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*rgb++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					*rgb++ = 0;
				}
	
				b += stepx;
			}

			// Skip the last pixel
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
			*rgb++ = 0;
		}
	}

	// Clean the last line
	memset(rgb, 0, nwidth * 4);
}


/** 
 * @brief This function permits to convert an image from bayer to BGR24
 *
 * @param bayer Buffer with the bayer data
 * @param image Size of image
 * @param view Size of view
 * @param hflip Horizontal flip
 * @param vflip Vertical flip
 * @param factor Factor of redimensioning
 *
 * @retval bgr Buffer with the BGR data
 */
void stk11xx_b2bgr24(uint8_t *bayer, uint8_t *bgr,
		struct stk11xx_coord *image,
		struct stk11xx_coord *view,
		const int hflip, const int vflip,
		const int factor) {
	uint8_t *b;

	int x, y; // Position in bayer image
	int i, j; // Position in bgr image

	int width = image->x;
	int height = image->y;

	int nwidth = width / factor;
	int nheight = height / factor;

	int offset;
	int startx, stepx;
	int starty, stepy;


	// Calculate the initial position (on Y axis)
	if (vflip) {
		starty = height - 2;
		stepy = -factor;
	}
	else {
		starty = 0;
		stepy = factor;
	}

	// Calculate the initial position (on X axis)
	if (hflip) {
		startx = width - 1;
		stepx = -factor;
		offset = width - 2;
	}
	else {
		startx = 0;
		stepx = factor;
		offset = 1;
	}


	// Skip the first line
	bayer += width;

	// To center vertically the image in the view
	bgr += ((view->y - nheight) / 2) * view->x * 3;

	// To center horizontally the image in the view
	bgr += ((view->x - nwidth) / 2) * 3;

	// Clean the first line
	memset(bgr, 0, nwidth * 3);
	bgr += nwidth * 3;


	// For each bgr line without the borders (first and last line)
	for (j=0, y=starty; j<nheight-2; j++, y=y+stepy) {
		// Go to the start of line
		b = bayer + y * width + offset;

		// Offset to center horizontally the image in the view
		bgr += (view->x - nwidth) * 3;

		if (y & 0x1) {
			// Skip the first pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;

			// GBGBGB : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*bgr++ = *b;
					*bgr++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*bgr++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
				}
				else {
					*bgr++ = (*(b-1) + *(b+1)) >> 1;
					*bgr++ = *b;
					*bgr++ = (*(b-width) + *(b+width)) >> 1;
				}

				b += stepx;
			}

			// Skip the last pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
		}
		else {
			// Skip the first pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;

			// RGRGRG : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*bgr++ = (*(b-width) + *(b+width)) >> 1;
					*bgr++ = *b;
					*bgr++ = (*(b-1) + *(b+1)) >> 1;
				}
				else {
					*bgr++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					*bgr++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*bgr++ = *b;
				}
	
				b += stepx;
			}

			// Skip the last pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
		}
	}

	// Clean the last line
	memset(bgr, 0, nwidth * 3);
}


/** 
 * @brief This function permits to convert an image from bayer to BGR32
 *
 * @param bayer Buffer with the bayer data
 * @param image Size of image
 * @param view Size of view
 * @param hflip Horizontal flip
 * @param vflip Vertical flip
 * @param factor Factor of redimensioning
 *
 * @retval bgr Buffer with the BGR data
 */
void stk11xx_b2bgr32(uint8_t *bayer, uint8_t *bgr,
		struct stk11xx_coord *image,
		struct stk11xx_coord *view,
		const int hflip, const int vflip,
		const int factor) {
	uint8_t *b;

	int x, y; // Position in bayer image
	int i, j; // Position in bgr image

	int width = image->x;
	int height = image->y;

	int nwidth = width / factor;
	int nheight = height / factor;

	int offset;
	int startx, stepx;
	int starty, stepy;


	// Calculate the initial position (on Y axis)
	if (vflip) {
		starty = height - 2;
		stepy = -factor;
	}
	else {
		starty = 0;
		stepy = factor;
	}

	// Calculate the initial position (on X axis)
	if (hflip) {
		startx = width - 1;
		stepx = -factor;
		offset = width - 2;
	}
	else {
		startx = 0;
		stepx = factor;
		offset = 1;
	}


	// Skip the first line
	bayer += width;

	// To center vertically the image in the view
	bgr += ((view->y - nheight) / 2) * view->x * 4;

	// To center horizontally the image in the view
	bgr += ((view->x - nwidth) / 2) * 4;

	// Clean the first line
	memset(bgr, 0, nwidth * 4);
	bgr += nwidth * 4;


	// For each bgr line without the borders (first and last line)
	for (j=0, y=starty; j<nheight-2; j++, y=y+stepy) {
		// Go to the start of line
		b = bayer + y * width + offset;

		// Offset to center horizontally the image in the view
		bgr += (view->x - nwidth) * 4;

		if (y & 0x1) {
			// Skip the first pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;

			// GBGBGB : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*bgr++ = *b;
					*bgr++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*bgr++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					*bgr++ = 0;
				}
				else {
					*bgr++ = (*(b-1) + *(b+1)) >> 1;
					*bgr++ = *b;
					*bgr++ = (*(b-width) + *(b+width)) >> 1;
					*bgr++ = 0;
				}

				b += stepx;
			}

			// Skip the last pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
		}
		else {
			// Skip the first pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;

			// RGRGRG : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					*bgr++ = (*(b-width) + *(b+width)) >> 1;
					*bgr++ = *b;
					*bgr++ = (*(b-1) + *(b+1)) >> 1;
					*bgr++ = 0;
				}
				else {
					*bgr++ = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					*bgr++ = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					*bgr++ = *b;
					*bgr++ = 0;
				}
	
				b += stepx;
			}

			// Skip the last pixel
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
			*bgr++ = 0;
		}
	}

	// Clean the last line
	memset(bgr, 0, nwidth * 4);
}


/** 
 * @brief This function permits to convert an image from bayer to YUV (UYVY)
 *
 * @param bayer Buffer with the bayer data
 * @param image Size of image
 * @param view Size of view
 * @param hflip Horizontal flip
 * @param vflip Vertical flip
 * @param factor Factor of redimensioning
 *
 * @retval yuv Buffer with the YUV data
 */
void stk11xx_b2uyvy(uint8_t *bayer, uint8_t *yuv,
		struct stk11xx_coord *image,
		struct stk11xx_coord *view,
		const int hflip, const int vflip,
		const int factor) {
	uint8_t *b;

	int x, y; // Position in bayer image
	int i, j; // Position in yuv image

	int pR, pG, pB;
	int pY, pU, pV;

	int width = image->x;
	int height = image->y;

	int nwidth = width / factor;
	int nheight = height / factor;

	int offset;
	int startx, stepx;
	int starty, stepy;


	// Calculate the initial position (on Y axis)
	if (vflip) {
		starty = height - 2;
		stepy = -factor;
	}
	else {
		starty = 0;
		stepy = factor;
	}

	// Calculate the initial position (on X axis)
	if (hflip) {
		startx = width - 1;
		stepx = -factor;
		offset = width - 2;
	}
	else {
		startx = 0;
		stepx = factor;
		offset = 1;
	}

	// Background color...
	memset(yuv, 16, width * 2);
	for (i=0; i<width*2; i=i+2, *(yuv+i)=128);
	for (i=1; i<height; i++)
		memcpy(yuv+i*width*2, yuv, width*2);

	// Skip the first line
	bayer += width;

	// To center vertically the image in the view
	yuv += ((view->y - nheight) / 2) * view->x * 2;

	// To center horizontally the image in the view
	yuv += ((view->x - nwidth) / 2) * 2;

	// Clean the first line
	memset(yuv, 16, nwidth * 2);
	for (i=0; i<nwidth*2; i=i+2, *(yuv+i)=128);
	yuv += nwidth * 2;


	// For each yuv line without the borders (first and last line)
	for (j=0, y=starty; j<nheight-2; j++, y=y+stepy) {
		// Go to the start of line
		b = bayer + y * width + offset;

		// Offset to center horizontally the image in the view
		yuv += (view->x - nwidth) * 2;

		if (y & 0x1) {
			// Skip the first pixel
			*yuv++ = 128;
			*yuv++ = 16;

			// GBGBGB : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					pR = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					pG = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					pB = *b;
				}
				else {
					pR = (*(b-width) + *(b+width)) >> 1;
					pG = *b;
					pB = (*(b-1) + *(b+1)) >> 1;
				}

				pY = stk11xx_yuv_interp[pR][0] + stk11xx_yuv_interp[pG][1] + stk11xx_yuv_interp[pB][2];
				pU = stk11xx_yuv_interp[pR][3] + stk11xx_yuv_interp[pG][4] + stk11xx_yuv_interp[pB][5];
				pV = stk11xx_yuv_interp[pR][5] + stk11xx_yuv_interp[pG][6] + stk11xx_yuv_interp[pB][7];

				pY = CLIP(pY, 0,255);
				pU = CLIP(pU, -127,127);
				pV = CLIP(pV, -127,127);
	
				if (i % 2){
					*yuv++ = (112 * pU)/127 + 128; // U
					*yuv++ = (219 * pY)/255 + 16;  // Y
				}
				else {
					*yuv++ = (112 * pV)/127 + 128; // V
					*yuv++ = (219 * pY)/255 + 16;  // Y
				}

				b += stepx;
			}

			// Skip the last pixel
			*yuv++ = 128;
			*yuv++ = 16;
		}
		else {
			// Skip the first pixel
			*yuv++ = 128;
			*yuv++ = 16;

			// RGRGRG : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					pR = (*(b-1) + *(b+1)) >> 1;
					pG = *b;
					pB = (*(b-width) + *(b+width)) >> 1;
				}
				else {
					pR = *b;
					pG = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					pB = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
				}

				pY = stk11xx_yuv_interp[pR][0] + stk11xx_yuv_interp[pG][1] + stk11xx_yuv_interp[pB][2];
				pU = stk11xx_yuv_interp[pR][3] + stk11xx_yuv_interp[pG][4] + stk11xx_yuv_interp[pB][5];
				pV = stk11xx_yuv_interp[pR][5] + stk11xx_yuv_interp[pG][6] + stk11xx_yuv_interp[pB][7];

				pY = CLIP(pY, 0,255);
				pU = CLIP(pU, -127,127);
				pV = CLIP(pV, -127,127);
	
				if (i % 2){
					*yuv++ = (112 * pU)/127 + 128; // U
					*yuv++ = (219 * pY)/255 + 16;  // Y
				}
				else {
					*yuv++ = (112 * pV)/127 + 128; // V
					*yuv++ = (219 * pY)/255 + 16;  // Y
				}

				b += stepx;
			}

			// Skip the last pixel
			*yuv++ = 128;
			*yuv++ = 16;
		}
	}

	// Clean the last line
	memset(yuv, 16, nwidth * 2);
	for (i=0; i<nwidth*2; i=i+2, *(yuv+i)=128);
}


/** 
 * @brief This function permits to convert an image from bayer to YUV (YUYV)
 *
 * @param bayer Buffer with the bayer data
 * @param image Size of image
 * @param view Size of view
 * @param hflip Horizontal flip
 * @param vflip Vertical flip
 * @param factor Factor of redimensioning
 *
 * @retval yuv Buffer with the YUV data
 */
void stk11xx_b2yuyv(uint8_t *bayer, uint8_t *yuv,
		struct stk11xx_coord *image,
		struct stk11xx_coord *view,
		const int hflip, const int vflip,
		const int factor) {
	uint8_t *b;

	int x, y; // Position in bayer image
	int i, j; // Position in yuv image

	int pR, pG, pB;
	int pY, pU, pV;

	int width = image->x;
	int height = image->y;

	int nwidth = width / factor;
	int nheight = height / factor;

	int offset;
	int startx, stepx;
	int starty, stepy;


	// Calculate the initial position (on Y axis)
	if (vflip) {
		starty = height - 2;
		stepy = -factor;
	}
	else {
		starty = 0;
		stepy = factor;
	}

	// Calculate the initial position (on X axis)
	if (hflip) {
		startx = width - 1;
		stepx = -factor;
		offset = width - 2;
	}
	else {
		startx = 0;
		stepx = factor;
		offset = 1;
	}

	// Background color...
	memset(yuv, 128, width * 2);
	for (i=0; i<width*2; i=i+2, *(yuv+i)=16);
	for (i=1; i<height; i++)
		memcpy(yuv+i*width*2, yuv, width*2);

	// Skip the first line
	bayer += width;

	// To center vertically the image in the view
	yuv += ((view->y - nheight) / 2) * view->x * 2;

	// To center horizontally the image in the view
	yuv += ((view->x - nwidth) / 2) * 2;

	// Clean the first line
	memset(yuv, 128, nwidth * 2);
	for (i=0; i<nwidth*2; i=i+2, *(yuv+i)=16);
	yuv += nwidth * 2;


	// For each yuv line without the borders (first and last line)
	for (j=0, y=starty; j<nheight-2; j++, y=y+stepy) {
		// Go to the start of line
		b = bayer + y * width + offset;

		// Offset to center horizontally the image in the view
		yuv += (view->x - nwidth) * 2;

		if (y & 0x1) {
			// Skip the first pixel
			*yuv++ = 16;
			*yuv++ = 128;

			// GBGBGB : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					pR = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
					pG = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					pB = *b;
				}
				else {
					pR = (*(b-width) + *(b+width)) >> 1;
					pG = *b;
					pB = (*(b-1) + *(b+1)) >> 1;
				}

				pY = stk11xx_yuv_interp[pR][0] + stk11xx_yuv_interp[pG][1] + stk11xx_yuv_interp[pB][2];
				pU = stk11xx_yuv_interp[pR][3] + stk11xx_yuv_interp[pG][4] + stk11xx_yuv_interp[pB][5];
				pV = stk11xx_yuv_interp[pR][5] + stk11xx_yuv_interp[pG][6] + stk11xx_yuv_interp[pB][7];

				pY = CLIP(pY, 0,255);
				pU = CLIP(pU, -127,127);
				pV = CLIP(pV, -127,127);
	
				if (i % 2){
					*yuv++ = (219 * pY)/255 + 16;  // Y
					*yuv++ = (112 * pU)/127 + 128; // U
				}
				else {
					*yuv++ = (219 * pY)/255 + 16;  // Y
					*yuv++ = (112 * pV)/127 + 128; // V
				}

				b += stepx;
			}

			// Skip the last pixel
			*yuv++ = 16;
			*yuv++ = 128;
		}
		else {
			// Skip the first pixel
			*yuv++ = 16;
			*yuv++ = 128;

			// RGRGRG : Line process...
			for (i=0, x=startx; i<nwidth-2; i++, x=x+stepx) {
				if (x & 0x1) {
					pR = (*(b-1) + *(b+1)) >> 1;
					pG = *b;
					pB = (*(b-width) + *(b+width)) >> 1;
				}
				else {
					pR = *b;
					pG = (*(b-width) + *(b-1) + *(b+1) + *(b+width)) >> 2;
					pB = (*(b-width-1) + *(b-width+1) + *(b+width-1) + *(b+width+1)) >> 2;
				}

				pY = stk11xx_yuv_interp[pR][0] + stk11xx_yuv_interp[pG][1] + stk11xx_yuv_interp[pB][2];
				pU = stk11xx_yuv_interp[pR][3] + stk11xx_yuv_interp[pG][4] + stk11xx_yuv_interp[pB][5];
				pV = stk11xx_yuv_interp[pR][5] + stk11xx_yuv_interp[pG][6] + stk11xx_yuv_interp[pB][7];

				pY = CLIP(pY, 0,255);
				pU = CLIP(pU, -127,127);
				pV = CLIP(pV, -127,127);
	
				if (i % 2){
					*yuv++ = (219 * pY)/255 + 16;  // Y
					*yuv++ = (112 * pU)/127 + 128; // U
				}
				else {
					*yuv++ = (219 * pY)/255 + 16;  // Y
					*yuv++ = (112 * pV)/127 + 128; // V
				}

				b += stepx;
			}

			// Skip the last pixel
			*yuv++ = 16;
			*yuv++ = 128;
		}
	}

	// Clean the last line
	memset(yuv, 128, nwidth * 2);
	for (i=0; i<nwidth*2; i=i+2, *(yuv+i)=16);
}


