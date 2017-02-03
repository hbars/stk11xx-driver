/** 
 * @file stk11xx-sysfs.c
 * @author Nicolas VIVIEN
 * @date 2006-10-23
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
 *   $Date$
 *   $Revision$
 *   $Author$
 *   $HeadURL$
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/kref.h>
#include <linux/device.h>
#include <linux/mm.h>
#if defined(VIDIOCGCAP)
#include <linux/videodev.h>
#endif


#include <linux/usb.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

#include "stk11xx.h"


extern const struct stk11xx_coord stk11xx_image_sizes[STK11XX_NBR_SIZES];


/** 
 * @brief show_release
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'release' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_release(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%d\n", dev->release);
}


/** 
 * @brief show_videostatus
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'videostatus' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_videostatus(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf,
			"Nbr ISOC errors    : %d\n"
			"Nbr dropped frames : %d\n"
			"Nbr dumped frames  : %d\n",
			dev->visoc_errors,
			dev->vframes_error,
			dev->vframes_dumped);
}


/** 
 * @brief show_informations
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'informations' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_informations(struct device *class, struct device_attribute *attr, char *buf)
{
	int width, height;
	char *pixelfmt = NULL;

	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	char *palette_rgb24 = "RGB24 - RGB-8-8-8 - 24 bits";
	char *palette_rgb32 = "RGB32 - RGB-8-8-8-8 - 32 bits";
	char *palette_bgr24 = "BGR24 - BGR-8-8-8 - 24 bits";
	char *palette_bgr32 = "BGR32 - BGR-8-8-8-8 - 32 bits";
	char *palette_uyvy = "UYVY - YUV 4:2:2 - 16 bits";
	char *palette_yuyv = "YUYV - YUV 4:2:2 - 16 bits";


	switch (dev->vsettings.palette) {
		case STK11XX_PALETTE_RGB24:
			pixelfmt = palette_rgb24;
			break;

		case STK11XX_PALETTE_RGB32:
			pixelfmt = palette_rgb32;
			break;

		case STK11XX_PALETTE_BGR24:
			pixelfmt = palette_bgr24;
			break;

		case STK11XX_PALETTE_BGR32:
			pixelfmt = palette_bgr32;
			break;

		case STK11XX_PALETTE_UYVY:
			pixelfmt = palette_uyvy;
			break;

		case STK11XX_PALETTE_YUYV:
			pixelfmt = palette_yuyv;
			break;
	}

	switch (dev->resolution) {
		case STK11XX_80x60:
		case STK11XX_128x96:
		case STK11XX_160x120:
		case STK11XX_213x160:
		case STK11XX_320x240:
		case STK11XX_640x480:
			width = stk11xx_image_sizes[STK11XX_640x480].x;
			height = stk11xx_image_sizes[STK11XX_640x480].y;
			break;
		case STK11XX_720x576:
			width = stk11xx_image_sizes[STK11XX_720x576].x;
			height = stk11xx_image_sizes[STK11XX_720x576].y;
			break;
		case STK11XX_800x600:
		case STK11XX_1024x768:
		case STK11XX_1280x1024:
			width = stk11xx_image_sizes[STK11XX_1280x1024].x;
			height = stk11xx_image_sizes[STK11XX_1280x1024].y;
			break;

		default:
			width = 0;
			height = 0;
	}

	return sprintf(buf,
			"Asked resolution  : %dx%d\n"
			"Driver resolution : %dx%d\n"
			"Webcam resolution : %dx%d\n"
			"\n"
			"%s\n"
			"\n"
			"Brightness        : 0x%X\n"
			"Contrast          : 0x%X\n"
			"Whiteness         : 0x%X\n"
			"Colour            : 0x%X\n",
			dev->view.x, dev->view.y,
			stk11xx_image_sizes[dev->resolution].x, stk11xx_image_sizes[dev->resolution].y,
			width, height,
			pixelfmt,
			0xFFFF & dev->vsettings.brightness,
			0xFFFF & dev->vsettings.contrast,
			0xFFFF & dev->vsettings.whiteness,
			0xFFFF & dev->vsettings.colour);
}


/** 
 * @brief show_fps
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'fps' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_fps(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%d\n", dev->vsettings.fps);
}


/** 
 * @brief show_brightness
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'brightness' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_brightness(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%X\n", dev->vsettings.brightness);
}


/** 
 * @brief store_brightness
 *
 * @param class Class device
 * @param buf Buffer
 * @param count Counter
 *
 * @returns Size of buffer
 */
static ssize_t store_brightness(struct device *class, struct device_attribute *attr, 
		const char *buf, size_t count)
{
	char *endp;
	unsigned long value;

	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	value = simple_strtoul(buf, &endp, 16);

	dev->vsettings.brightness = (int) value;

	dev_stk11xx_set_camera_quality(dev);

	return strlen(buf);
}

/** 
 * @brief show_contrast
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'contrast' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_contrast(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%X\n", dev->vsettings.contrast);
}


/** 
 * @brief store_contrast
 *
 * @param class Class device
 * @param buf Buffer
 * @param count Counter
 *
 * @returns Size of buffer
 */
static ssize_t store_contrast(struct device *class, struct device_attribute *attr,
		const char *buf, size_t count)
{
	char *endp;
	unsigned long value;

	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	value = simple_strtoul(buf, &endp, 16);

	dev->vsettings.contrast = (int) value;

	dev_stk11xx_set_camera_quality(dev);

	return strlen(buf);
}


/** 
 * @brief show_whitebalance
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'whitebalance' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_whitebalance(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%X\n", dev->vsettings.whiteness);
}


/** 
 * @brief store_whitebalance
 *
 * @param class Class device
 * @param buf Buffer
 * @param count Counter
 *
 * @returns Size of buffer
 */
static ssize_t store_whitebalance(struct device *class, struct device_attribute *attr,
		const char *buf, size_t count)
{
	char *endp;
	unsigned long value;

	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	value = simple_strtoul(buf, &endp, 16);

	dev->vsettings.whiteness = (int) value;

	dev_stk11xx_set_camera_quality(dev);

	return strlen(buf);
}


/** 
 * @brief show_colour
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'colour' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_colour(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%X\n", dev->vsettings.colour);
}


/** 
 * @brief store_colour
 *
 * @param class Class device
 * @param buf Buffer
 * @param count Counter
 *
 * @returns Size of buffer
 */
static ssize_t store_colour(struct device *class, struct device_attribute *attr,
		const char *buf, size_t count)
{
	char *endp;
	unsigned long value;

	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	value = simple_strtoul(buf, &endp, 16);

	dev->vsettings.colour = (int) value;

	dev_stk11xx_set_camera_quality(dev);

	return strlen(buf);
}


/** 
 * @brief show_hflip
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'hflip' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_hflip(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%d\n", dev->vsettings.hflip);
}


/** 
 * @brief store_hflip
 *
 * @param class Class device
 * @param buf Buffer
 * @param count Counter
 *
 * @returns Size of buffer
 */
static ssize_t store_hflip(struct device *class, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	if (strncmp(buf, "1", 1) == 0)
		dev->vsettings.hflip = 1;
	else if (strncmp(buf, "0", 1) == 0)
		dev->vsettings.hflip = 0;
	else
		return -EINVAL;

	return strlen(buf);
}


/** 
 * @brief show_vflip
 *
 * @param class Class device
 *
 * @retval buf Adress of buffer with the 'vflip' value
 * 
 * @returns Size of buffer
 */
static ssize_t show_vflip(struct device *class, struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	return sprintf(buf, "%d\n", dev->vsettings.vflip);
}


/** 
 * @brief store_vflip
 *
 * @param class Class device
 * @param buf Buffer
 * @param count Counter
 *
 * @returns Size of buffer
 */
static ssize_t store_vflip(struct device *class, struct device_attribute *attr, const char *buf, size_t count)
{
	struct video_device *vdev = to_video_device(class);
	struct usb_stk11xx *dev = video_get_drvdata(vdev);

	if (strncmp(buf, "1", 1) == 0)
		dev->vsettings.vflip = 1;
	else if (strncmp(buf, "0", 1) == 0)
		dev->vsettings.vflip = 0;
	else
		return -EINVAL;

	return strlen(buf);
}


static DEVICE_ATTR(release, S_IRUGO, show_release, NULL);											/**< Release value */
static DEVICE_ATTR(videostatus, S_IRUGO, show_videostatus, NULL);									/**< Video status */
static DEVICE_ATTR(informations, S_IRUGO, show_informations, NULL);									/**< Informations */
static DEVICE_ATTR(fps, S_IRUGO, show_fps, NULL);													/**< FPS value */
static DEVICE_ATTR(brightness, 0660, show_brightness, store_brightness);				/**< Brightness value */
static DEVICE_ATTR(contrast, 0660, show_contrast, store_contrast);						/**< Contrast value */
static DEVICE_ATTR(whitebalance, 0660, show_whitebalance, store_whitebalance);			/**< Whitebalance value */
static DEVICE_ATTR(colour, 0660, show_colour, store_colour);							/**< Hue value */
static DEVICE_ATTR(hflip, 0660, show_hflip, store_hflip);								/**< Horizontal filp value */
static DEVICE_ATTR(vflip, 0660, show_vflip, store_vflip);								/**< Vertical filp value */


/** 
 * @brief Create the 'sys' entries.
 *
 * This function permits to create all the entries in the 'sys' filesystem.
 *
 * @param vdev Video device structure
 * 
 * @returns 0 if all is OK
 */
int stk11xx_create_sysfs_files(struct video_device *vdev)
{
	int ret;

	ret = device_create_file(&vdev->dev, &dev_attr_release);
	ret = device_create_file(&vdev->dev, &dev_attr_videostatus);
	ret = device_create_file(&vdev->dev, &dev_attr_informations);
	ret = device_create_file(&vdev->dev, &dev_attr_fps);
	ret = device_create_file(&vdev->dev, &dev_attr_brightness);
	ret = device_create_file(&vdev->dev, &dev_attr_contrast);
	ret = device_create_file(&vdev->dev, &dev_attr_whitebalance);
	ret = device_create_file(&vdev->dev, &dev_attr_colour);
	ret = device_create_file(&vdev->dev, &dev_attr_hflip);
	ret = device_create_file(&vdev->dev, &dev_attr_vflip);

	return ret;
}


/** 
 * @brief Remove the 'sys' entries.
 *
 * This function permits to remove all the entries in the 'sys' filesystem.
 *
 * @param vdev Video device structure
 * 
 * @returns 0 if all is OK
 */
void stk11xx_remove_sysfs_files(struct video_device *vdev)
{
	device_remove_file(&vdev->dev, &dev_attr_release);
	device_remove_file(&vdev->dev, &dev_attr_videostatus);
	device_remove_file(&vdev->dev, &dev_attr_informations);
	device_remove_file(&vdev->dev, &dev_attr_fps);
	device_remove_file(&vdev->dev, &dev_attr_brightness);
	device_remove_file(&vdev->dev, &dev_attr_contrast);
	device_remove_file(&vdev->dev, &dev_attr_whitebalance);
	device_remove_file(&vdev->dev, &dev_attr_colour);
	device_remove_file(&vdev->dev, &dev_attr_hflip);
	device_remove_file(&vdev->dev, &dev_attr_vflip);
}

