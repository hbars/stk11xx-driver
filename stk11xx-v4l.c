/** 
 * @file stk11xx-v4l.c
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
#include <linux/vmalloc.h>
#include <linux/mm.h>
#if defined(VIDIOCGCAP)
#include <linux/videodev.h>
#endif


#include <linux/usb.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

#include "stk11xx.h"


static struct v4l2_file_operations v4l_stk11xx_fops;


/**
 * @var stk11xx_image_sizes
 *   List of all resolutions supported by the driver
 */
const struct stk11xx_coord stk11xx_image_sizes[STK11XX_NBR_SIZES] = {
	{   80,   60 },
	{  128,   96 },
	{  160,  120 },
	{  213,  160 },
	{  320,  240 },
	{  640,  480 },
	{  720,  576 },
	{  800,  600 },
	{ 1024,  768 },
	{ 1280, 1024 }
};


/**
 * @var stk11xx_controls
 *   List of all V4Lv2 controls supported by the driver
 *   default_value field will be overridden at runtime
 */
static struct v4l2_queryctrl stk11xx_controls[] = {
	{
		.id      = V4L2_CID_BRIGHTNESS,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Brightness",
		.minimum = 0,
		.maximum = 0xff00,
		.step    = 1,
		.default_value = 0x7f00,
	},
	{
		.id      = V4L2_CID_WHITENESS,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Whiteness",
		.minimum = 0,
		.maximum = 0xff00,
		.step    = 1,
		.default_value = 0x7f00,
	},
	{
		.id      = V4L2_CID_SATURATION,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Saturation",
		.minimum = 0,
		.maximum = 0xff00,
		.step    = 1,
		.default_value = 0x7f00,
	},
	{
		.id      = V4L2_CID_CONTRAST,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Contrast",
		.minimum = 0,
		.maximum = 0xff00,
		.step    = 1,
		.default_value = 0x7f00,
	},
	{
		.id      = V4L2_CID_HUE,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Hue",
		.minimum = 0,
		.maximum = 0xff00,
		.step    = 1,
		.default_value = 0x7f00,
	},
	{
		.id      = V4L2_CID_HFLIP,
		.type    = V4L2_CTRL_TYPE_BOOLEAN,
		.name    = "Flip Horizontally",
		.minimum = 0,
		.maximum = 1,
		.step    = 1,
		.default_value = 0, // will be actually set later
	},
	{
		.id      = V4L2_CID_VFLIP,
		.type    = V4L2_CTRL_TYPE_BOOLEAN,
		.name    = "Flip Vertically",
		.minimum = 0,
		.maximum = 1,
		.step    = 1,
		.default_value = 0, // will be actually set later
	}
};


/** 
 * @param dev
 * @param width Width of wished resolution
 * @param height Height of wished resolution
 * 
 * @returns 0 if all is OK
 *
 * @brief Select a video mode
 *
 * This function permits to check and select a video mode.
 */
int v4l_stk11xx_select_video_mode(struct usb_stk11xx *dev, int width, int height)
{
	int i;
	int find;


	// Check width and height
	// Notice : this test is usefull for the Kopete application !

	// Driver can't build an image smaller than the minimal resolution !
	if ((width < stk11xx_image_sizes[0].x)
			|| (height < stk11xx_image_sizes[0].y)) {
		width = stk11xx_image_sizes[0].x;
		height = stk11xx_image_sizes[0].y;
	}

	// Driver can't build an image bigger than the maximal resolution !
	switch (dev->webcam_type) {
		case STK11XX_SXGA:
			if ((width > stk11xx_image_sizes[STK11XX_1280x1024].x)
				|| (height > stk11xx_image_sizes[STK11XX_1280x1024].y)) {
				width = stk11xx_image_sizes[STK11XX_1280x1024].x;
				height = stk11xx_image_sizes[STK11XX_1280x1024].y;
			}
			break;

		case STK11XX_VGA:
			if ((width > stk11xx_image_sizes[STK11XX_640x480].x)
				|| (height > stk11xx_image_sizes[STK11XX_640x480].y)) {
				width = stk11xx_image_sizes[STK11XX_640x480].x;
				height = stk11xx_image_sizes[STK11XX_640x480].y;
			}
			break;

		case STK11XX_PAL:
			if (! (((width == 720) && (height==576))
				|| ((width == 720) && (height==480))
				|| ((width == 640) && (height==480)))) {
				width = 640;
				height = 480;
			}
			break;

		default:
			return -1;
	}


	// Seek the best resolution
	switch (dev->webcam_type) {
		case STK11XX_SXGA:
			for (i=0, find=0; i<=STK11XX_1280x1024; i++) {
				if (stk11xx_image_sizes[i].x <= width && stk11xx_image_sizes[i].y <= height)
					find = i;
			}
			break;

		case STK11XX_VGA:
			for (i=0, find=0; i<=STK11XX_640x480; i++) {
				if (stk11xx_image_sizes[i].x <= width && stk11xx_image_sizes[i].y <= height)
					find = i;
			}
			break;

		case STK11XX_PAL:
			for (i=0, find=0; i<=STK11XX_720x576; i++) {
				if (stk11xx_image_sizes[i].x <= width && stk11xx_image_sizes[i].y <= height)
					find = i;
			}
			break;

		default:
			return -1;
	}

	// Save the new resolution
	dev->resolution = find;

	STK_DEBUG("Set mode %d [%dx%d]\n", dev->resolution,
			stk11xx_image_sizes[dev->resolution].x, stk11xx_image_sizes[dev->resolution].y);

	// Save the new size
	dev->view.x = width;
	dev->view.y = height;


	// Calculate the frame size
	if (dev->webcam_type == STK11XX_PAL) {
		// Here, dev->resolution equals : 640x480 || 720x576
		dev->image.x = stk11xx_image_sizes[dev->resolution].x;
		dev->image.y = stk11xx_image_sizes[dev->resolution].y;
		dev->frame_size = dev->image.x * dev->image.y;
	}
	else {
		switch (dev->resolution) {
			case STK11XX_80x60:
			case STK11XX_128x96:
			case STK11XX_160x120:
			case STK11XX_213x160:
			case STK11XX_320x240:
			case STK11XX_640x480:
				dev->image.x = stk11xx_image_sizes[STK11XX_640x480].x;
				dev->image.y = stk11xx_image_sizes[STK11XX_640x480].y;
				dev->frame_size = dev->image.x * dev->image.y;
				break;

			case STK11XX_720x576:
			case STK11XX_800x600:
			case STK11XX_1024x768:
			case STK11XX_1280x1024:
				dev->image.x = stk11xx_image_sizes[STK11XX_1280x1024].x;
				dev->image.y = stk11xx_image_sizes[STK11XX_1280x1024].y;
				dev->frame_size = dev->image.x * dev->image.y;
				break;
		}
	}


	// Calculate the image size
	switch (dev->vsettings.palette) {
		case STK11XX_PALETTE_RGB24:
		case STK11XX_PALETTE_BGR24:
			dev->view_size = 3 * dev->view.x * dev->view.y;
			dev->image_size = 3 * dev->frame_size;
			break;

		case STK11XX_PALETTE_RGB32:
		case STK11XX_PALETTE_BGR32:
			dev->view_size = 3 * dev->view.x * dev->view.y;
			dev->image_size = 4 * dev->frame_size;
			break;

		case STK11XX_PALETTE_UYVY:
		case STK11XX_PALETTE_YUYV:
			dev->view_size = 2 * dev->view.x * dev->view.y;
			dev->image_size = 2 * dev->frame_size;
			break;
	}

	return 0;
}


/** 
 * @param fp File pointer
 * 
 * @returns 0 if all is OK
 *
 * @brief Open the video device
 *
 * This function permits to open a video device (/dev/videoX)
 */
static int v4l_stk11xx_open(struct file *fp)
{
	int err;

	struct usb_stk11xx *dev;
	struct video_device *vdev;
	
	vdev = video_devdata(fp);
	dev = video_get_drvdata(video_devdata(fp));

	if (dev == NULL) {
		STK_ERROR("Device not initialized !!!\n");
		BUG();
	}

	mutex_lock(&dev->modlock);

	if (dev->vopen) {
		STK_DEBUG("Device is busy, someone is using the device\n");
		mutex_unlock(&dev->modlock);
		return -EBUSY;
	}

	// Allocate memory
	err = stk11xx_allocate_buffers(dev);

	if (err < 0) {
		STK_ERROR("Failed to allocate buffer memory !\n");
		mutex_unlock(&dev->modlock);
		return err;
	}
	
	// Reset buffers and parameters
	stk11xx_reset_buffers(dev);

	// Settings
	dev->vsync = 0;
	dev->v1st_cap = 5;
	dev->error_status = 0;
	dev->visoc_errors = 0;
	dev->vframes_error = 0;
	dev->vframes_dumped = 0;
	dev->vsettings.hue = 0xffff;
	dev->vsettings.whiteness = 0xffff;
	dev->vsettings.depth = 24;
	dev->vsettings.palette = STK11XX_PALETTE_BGR24;

	// Select the resolution by default
	v4l_stk11xx_select_video_mode(dev, 640, 480);

	// Initialize the device
	dev_stk11xx_init_camera(dev);
	dev_stk11xx_camera_on(dev);
	dev_stk11xx_reconf_camera(dev);

	// Init Isoc and URB
	err = usb_stk11xx_isoc_init(dev);

	if (err) {
		STK_ERROR("Failed to init ISOC stuff !\n");
		usb_stk11xx_isoc_cleanup(dev);
		stk11xx_free_buffers(dev);
		mutex_unlock(&dev->modlock);
		return err;
	}

	// Start the video stream
	dev_stk11xx_start_stream(dev);

	// Video settings
	dev_stk11xx_camera_settings(dev);

	// Register interface on power management
	usb_autopm_get_interface(dev->interface);

	dev->vopen++;
	fp->private_data = vdev;

	mutex_unlock(&dev->modlock);

	return 0;
}


/** 
 * @param fp File pointer
 * 
 * @returns 0 if all is OK
 *
 * @brief Release an opened file.
 *
 * This function permits to release an opened file with the 'open' method.
 */
static int v4l_stk11xx_release(struct file *fp)
{
	struct usb_stk11xx *dev;
	struct video_device *vdev;
	
	vdev = video_devdata(fp);
	dev = video_get_drvdata(video_devdata(fp));

	if (dev->vopen == 0)
		STK_ERROR("v4l_release called on closed device\n");

	// Stop the video stream
	dev_stk11xx_stop_stream(dev);

	// ISOC and URB cleanup
	usb_stk11xx_isoc_cleanup(dev);

	// Free memory
	stk11xx_free_buffers(dev);

	// Switch off the camera
	dev_stk11xx_camera_off(dev);

	dev_stk11xx_camera_asleep(dev);

	// Unregister interface on power management
	usb_autopm_put_interface(dev->interface);

	dev->vopen--;

	return 0;
}


/** 
 * @param fp File pointer
 *
 * @retval buf Buffer in user space
 * @retval count 
 * @retval f_pos 
 * 
 * @returns Count value
 *
 * @brief Read the video device
 *
 * This function is called by the application is reading the video device.
 */
static ssize_t v4l_stk11xx_read(struct file *fp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	int noblock = fp->f_flags & O_NONBLOCK;

	struct usb_stk11xx *dev;
	struct video_device *vdev;

	int bytes_to_read;
	void *image_buffer_addr;
	
	DECLARE_WAITQUEUE(wait, current);

	vdev = video_devdata(fp);
	dev = video_get_drvdata(video_devdata(fp));

	STK_STREAM("Read vdev=0x%p, buf=0x%p, count=%zd\n", vdev, buf, count);

	if (dev == NULL)
		return -EFAULT;

	if (vdev == NULL)
		return -EFAULT;

	mutex_lock(&dev->modlock);

	if (dev->image_read_pos == 0) {
		add_wait_queue(&dev->wait_frame, &wait);

		while (dev->full_frames == NULL) {
			if (dev->error_status) {
				remove_wait_queue(&dev->wait_frame, &wait);
				set_current_state(TASK_RUNNING);
				mutex_unlock(&dev->modlock);
				return -dev->error_status ;
			}

			if (noblock) {
				remove_wait_queue(&dev->wait_frame, &wait);
				set_current_state(TASK_RUNNING);
				mutex_unlock(&dev->modlock);
				return -EWOULDBLOCK;
			}

			if (signal_pending(current)) {
				remove_wait_queue(&dev->wait_frame, &wait);
				set_current_state(TASK_RUNNING);
				mutex_unlock(&dev->modlock);
				return -ERESTARTSYS;
			}

			schedule();
			set_current_state(TASK_INTERRUPTIBLE);
		}

		remove_wait_queue(&dev->wait_frame, &wait);
		set_current_state(TASK_RUNNING);

		if (stk11xx_handle_frame(dev)) {
			mutex_unlock(&dev->modlock);
			return -EFAULT;
		}
	}

	bytes_to_read = dev->view_size;

	if (count + dev->image_read_pos > bytes_to_read)
		count = bytes_to_read - dev->image_read_pos;

	image_buffer_addr = dev->image_data;
	image_buffer_addr += dev->images[dev->fill_image].offset;
	image_buffer_addr += dev->image_read_pos;

	if (copy_to_user(buf, image_buffer_addr, count)) {
		mutex_unlock(&dev->modlock);
		return -EFAULT;
	}
	
	dev->image_read_pos += count;
	
	if (dev->image_read_pos >= bytes_to_read) {
		dev->image_read_pos = 0;
		stk11xx_next_image(dev);
	}

	mutex_unlock(&dev->modlock);

	return count;
}


/** 
 * @param fp File pointer
 * @param wait 
 * 
 * @returns 0 if all is OK
 *
 * @brief Polling function
 */
static unsigned int v4l_stk11xx_poll(struct file *fp, poll_table *wait)
{
	struct usb_stk11xx *dev;
	struct video_device *vdev;
	
	vdev = video_devdata(fp);
	dev = video_get_drvdata(video_devdata(fp));

	STK_STREAM("Poll\n");

	if (vdev == NULL)
		return -EFAULT;

	if (dev == NULL)
		return -EFAULT;

	poll_wait(fp, &dev->wait_frame, wait);

	if (dev->error_status)
		return POLLERR;

	if (dev->full_frames != NULL)
		return (POLLIN | POLLRDNORM);

	return 0;
}


/** 
 * @param fp File pointer
 * @param vma VMA structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Memory map
 *
 * This function permits to map a memory space.
 */
static int v4l_stk11xx_mmap(struct file *fp, struct vm_area_struct *vma)
{
	unsigned int i;

	unsigned long size;
	unsigned long start;
	unsigned long pos;
	unsigned long page;

	struct usb_stk11xx *dev;

	struct video_device *vdev;
	
	vdev = video_devdata(fp);
	dev = video_get_drvdata(video_devdata(fp));

	STK_STREAM("mmap\n");

	start = vma->vm_start;
	size = vma->vm_end - vma->vm_start;

	// Find the buffer for this mapping...
	for (i=0; i<dev->nbuffers; i++) {
		pos = dev->images[i].offset;

		if ((pos >> PAGE_SHIFT) == vma->vm_pgoff)
			break;
	}

	// If no buffer found !
	if (i == STK11XX_MAX_IMAGES) {
		STK_ERROR("mmap no buffer found !\n");
		return -EINVAL;
	}

	if (i == 0) {
		unsigned long total_size;

		total_size = dev->nbuffers * dev->len_per_image;

		if (size != dev->len_per_image && size != total_size) {
			STK_ERROR("Wrong size (%lu) needed to be len_per_image=%d or total_size=%lu\n",
				size, dev->len_per_image, total_size);
				
			return -EINVAL;
		}
	}
	else if (size > dev->len_per_image)
		return -EINVAL;

	vma->vm_flags |= VM_IO;

	pos = (unsigned long) dev->image_data;

	while (size > 0) {
		page = vmalloc_to_pfn((void *) pos);

		if (remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED))
			return -EAGAIN;

		start += PAGE_SIZE;
		pos += PAGE_SIZE;

		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}

	return 0;
}


/** 
 * @param fp File pointer
 * @param cmd Command
 * @param arg Arguments of the command
 * 
 * @returns 0 if all is OK
 *
 * @brief Manage IOCTL
 *
 * This function permits to manage all the IOCTL from the application.
 */
static long v4l_stk11xx_do_ioctl(struct file *fp,
		unsigned int cmd, void __user *arg)
{
	struct usb_stk11xx *dev;
	struct video_device *vdev;

	DECLARE_WAITQUEUE(wait, current);
	
	vdev = video_devdata(fp);
	dev = video_get_drvdata(video_devdata(fp));

#if (CONFIG_STK11XX_DEBUG == 1)
	v4l_printk_ioctl(cmd);
#endif

	switch (cmd) {
#ifdef VIDIOCGCAP
		// Video 4 Linux v1

		case VIDIOCGCAP:
			{
				struct video_capability *cap = arg;

				STK_DEBUG("VIDIOCGCAP\n");

				memset(cap, 0, sizeof(*cap));
				strlcpy(cap->name, "stk11xx", sizeof(cap->name));
				cap->type = VID_TYPE_CAPTURE;
				cap->channels = 1;
				cap->audios = 0;

				switch (dev->webcam_type) {
					case STK11XX_SXGA:
						cap->minwidth = stk11xx_image_sizes[STK11XX_80x60].x;
						cap->minheight = stk11xx_image_sizes[STK11XX_80x60].y;
						cap->maxwidth = stk11xx_image_sizes[STK11XX_1280x1024].x;
						cap->maxheight = stk11xx_image_sizes[STK11XX_1280x1024].y;
						break;

					case STK11XX_PAL:
						cap->minwidth = stk11xx_image_sizes[STK11XX_640x480].x;
						cap->minheight = stk11xx_image_sizes[STK11XX_640x480].y;
						cap->maxwidth = stk11xx_image_sizes[STK11XX_720x576].x;
						cap->maxheight = stk11xx_image_sizes[STK11XX_720x576].y;
						break;
						
					case STK11XX_VGA:
						cap->minwidth = stk11xx_image_sizes[STK11XX_80x60].x;
						cap->minheight = stk11xx_image_sizes[STK11XX_80x60].y;
						cap->maxwidth = stk11xx_image_sizes[STK11XX_640x480].x;
						cap->maxheight = stk11xx_image_sizes[STK11XX_640x480].y;
						break;
				}
			}
			break;
	
		case VIDIOCGCHAN:
			{
			    struct video_channel *v = arg;

				STK_DEBUG("VIDIOCGCHAN\n");

			    if (v->channel != 0)
				    return -EINVAL;
			
				v->flags = 0;
			    v->tuners = 0;
			    v->type = VIDEO_TYPE_CAMERA;
			    strcpy(v->name, "Webcam");
			}
			break;

		case VIDIOCSCHAN:
			{
				struct video_channel *v = arg;

				STK_DEBUG("VIDIOCSCHAN\n");

				if (v->channel != 0)
					return -EINVAL;
			}
			break;

		case VIDIOCGPICT:
			{
				struct video_picture *p = arg;

				STK_DEBUG("VIDIOCGPICT\n");

				p->brightness = dev->vsettings.brightness;
				p->contrast = dev->vsettings.contrast;
				p->whiteness = dev->vsettings.whiteness;
				p->colour = dev->vsettings.colour;
				p->depth = dev->vsettings.depth;
				p->palette = dev->vsettings.palette;
				p->hue = dev->vsettings.hue;

				switch (dev->vsettings.palette) {
					case STK11XX_PALETTE_BGR24:
						p->palette = VIDEO_PALETTE_RGB24;
						break;

					case STK11XX_PALETTE_BGR32:
						p->palette = VIDEO_PALETTE_RGB32;
						break;

					case STK11XX_PALETTE_UYVY:
						p->palette = VIDEO_PALETTE_UYVY;
						break;

					case STK11XX_PALETTE_YUYV:
						p->palette = VIDEO_PALETTE_YUYV;
						break;
				}
			}
			break;

		case VIDIOCSPICT:
			{
				struct video_picture *p = arg;

				STK_DEBUG("VIDIOCSPICT\n");

				dev->vsettings.brightness = p->brightness;
				dev->vsettings.contrast = p->contrast;
				dev->vsettings.whiteness = p->whiteness;
				dev->vsettings.colour = p->colour;
				dev->vsettings.hue = p->hue;
				
				if (p->palette && p->palette != dev->vsettings.palette) {
					switch (p->palette) {
						case VIDEO_PALETTE_RGB24:
							dev->vsettings.depth = 24;
							dev->vsettings.palette = STK11XX_PALETTE_BGR24;
							break;

						case VIDEO_PALETTE_RGB32:
							dev->vsettings.depth = 32;
							dev->vsettings.palette = STK11XX_PALETTE_BGR32;
							break;

						case VIDEO_PALETTE_UYVY:
							dev->vsettings.depth = 16;
							dev->vsettings.palette = STK11XX_PALETTE_UYVY;
							break;

						case VIDEO_PALETTE_YUYV:
							dev->vsettings.depth = 16;
							dev->vsettings.palette = STK11XX_PALETTE_YUYV;
							break;

						default:
							return -EINVAL;
					}
				}

				dev_stk11xx_camera_settings(dev);

				STK_DEBUG("VIDIOCSPICT done\n");
			}
			break;

		case VIDIOCGWIN:
			{
				struct video_window *vw = arg;

				STK_DEBUG("VIDIOCGWIN\n");

				vw->x = 0;
				vw->y = 0;
				vw->width = dev->view.x;
				vw->height = dev->view.y;
				vw->chromakey = 0;
			}
			break;

		case VIDIOCSWIN:
			{
				struct video_window *vw = arg;

				STK_DEBUG("VIDIOCSWIN\n");

				STK_DEBUG("Set x=%d, y=%d\n", vw->x, vw->y);
				STK_DEBUG("Set width=%d, height=%d\n", vw->width, vw->height);
				STK_DEBUG("Flags = %X\n", vw->flags);

				// Stop the video stream
				dev_stk11xx_stop_stream(dev);
			
				// ISOC and URB cleanup
				usb_stk11xx_isoc_cleanup(dev);

				// Switch off the camera
				dev_stk11xx_camera_off(dev);

				dev_stk11xx_camera_asleep(dev);

				// Select the new video mode
				if (v4l_stk11xx_select_video_mode(dev, vw->width, vw->height)) {
					STK_ERROR("Select video mode failed !\n");
					return -EAGAIN;
				}

				// Clear the buffers
				stk11xx_clear_buffers(dev);

				// Initialize the device
				dev_stk11xx_init_camera(dev);
				dev_stk11xx_camera_on(dev);
				dev_stk11xx_reconf_camera(dev);

				// ISOC and URB init
				usb_stk11xx_isoc_init(dev);

				// Re-start the stream
				dev_stk11xx_start_stream(dev);

				// Video settings
				dev_stk11xx_camera_settings(dev);
			}
			break;

		case VIDIOCGFBUF:
			{
				struct video_buffer *vb = arg;

				STK_DEBUG("VIDIOCGFBUF\n");

				memset(vb, 0, sizeof(*vb));
			}
			break;

		case VIDIOCGMBUF:
			{
				int i;
				struct video_mbuf *vm = arg;

				STK_DEBUG("VIDIOCGMBUF\n");

				memset(vm, 0, sizeof(*vm));

				vm->size = dev->nbuffers * dev->len_per_image;
				vm->frames = dev->nbuffers;

				for (i=0; i<dev->nbuffers; i++)
					vm->offsets[i] = i * dev->len_per_image;
			}
			break;

		case VIDIOCMCAPTURE:
			{
				struct video_mmap *vm = arg;

				STK_DEBUG("VIDIOCMCAPTURE format=%d\n", vm->format);

				if (vm->frame < 0 || vm->frame >= dev->nbuffers)
					return -EINVAL;

				if (vm->format) {
					switch (vm->format) {
						case VIDEO_PALETTE_RGB32:
							break;

						case VIDEO_PALETTE_RGB24:
							break;

						case VIDEO_PALETTE_UYVY:
							break;

						case VIDEO_PALETTE_YUYV:
							break;

						default:
							return -EINVAL;
					}
				}

				if ((vm->width != dev->view.x) || (vm->height != dev->view.y)) 
					return -EAGAIN;

				if (dev->image_used[vm->frame])
					return -EBUSY;

				dev->image_used[vm->frame] = 1;

				STK_DEBUG("VIDIOCMCAPTURE done\n");
			}
			break;

		case VIDIOCSYNC:
			{
				int ret;
				int *mbuf = arg;

				STK_DEBUG("VIDIOCSYNC\n");

				if (*mbuf < 0 || *mbuf >= dev->nbuffers)
					return -EINVAL;

				if (dev->image_used[*mbuf] == 0)
					return -EINVAL;

				add_wait_queue(&dev->wait_frame, &wait);

				while (dev->full_frames == NULL) {
					if (dev->error_status) {
						remove_wait_queue(&dev->wait_frame, &wait);
						set_current_state(TASK_RUNNING);
						return -dev->error_status;
					}

					if (signal_pending(current)) {
						remove_wait_queue(&dev->wait_frame, &wait);
						set_current_state(TASK_RUNNING);
						return -ERESTARTSYS;
					}

					schedule();
					set_current_state(TASK_INTERRUPTIBLE);
				}

				remove_wait_queue(&dev->wait_frame, &wait);
				set_current_state(TASK_RUNNING);

				STK_DEBUG("VIDIOCSYNC: frame ready\n");

				dev->fill_image = *mbuf;

				ret = stk11xx_handle_frame(dev);

				if (ret != 0)
					STK_ERROR("VIDIOCSYNC error !\n");

				dev->image_used[*mbuf] = 0;
			}
			break;

		case VIDIOCGAUDIO:
			STK_DEBUG("VIDIOCGAUDIO\n");
			return -EINVAL;
			break;

		case VIDIOCSAUDIO:
			STK_DEBUG("VIDIOCSAUDIO\n");
			return -EINVAL;
			break;

		case VIDIOCGUNIT:
			{
				struct video_unit *vu = arg;

				vu->video = dev->vdev->minor & 0x3f;
				vu->audio = -1;
				vu->vbi = -1;
				vu->radio = -1;
				vu->teletext = -1;
			}
			break;
#endif /* VIDIOCGCAP */


		// Video 4 Linux v2

		case VIDIOC_QUERYCAP:
			{
				struct v4l2_capability *cap = arg;

				STK_DEBUG("VIDIOC_QUERYCAP\n");

				memset(cap, 0, sizeof(*cap));
				strlcpy(cap->driver, "stk11xx", sizeof(cap->driver));

				cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE | V4L2_CAP_STREAMING;
				cap->version = (__u32) DRIVER_VERSION_NUM, strlcpy(cap->card, dev->vdev->name, sizeof(cap->card));
			
				if (usb_make_path(dev->udev, cap->bus_info, sizeof(cap->bus_info)) < 0)
					strlcpy(cap->bus_info, dev->vdev->name, sizeof(cap->bus_info));
			}
			break;

		case VIDIOC_ENUMINPUT:
			{
				struct v4l2_input *i = arg;

				STK_DEBUG("VIDIOC_ENUMINPUT %d\n", i->index);

				if (dev->webcam_model != SYNTEK_STK_0408) {
					if (i->index)
						return -EINVAL;
					strlcpy(i->name, "USB", sizeof(i->name));
				}
				else {
					if (i->index > 3)
						return -EINVAL;
					
					switch (i->index) {
						case 0:
							strlcpy(i->name, "Input1", sizeof(i->name));
							break;
						case 1:
							strlcpy(i->name, "Input2", sizeof(i->name));
							break;
						case 2:
							strlcpy(i->name, "Input3", sizeof(i->name));
							break;
						case 3:
							strlcpy(i->name, "Input4", sizeof(i->name));
							break;
					}
				}
				
				i->type = V4L2_INPUT_TYPE_CAMERA;
			}
			break;

		case VIDIOC_G_INPUT:
			{
				STK_DEBUG("GET INPUT\n");

				return dev->vsettings.input;
			}
			break;

		case VIDIOC_S_INPUT:
			{
				struct v4l2_input *i = arg;

				STK_DEBUG("SET INPUT %d\n", i->index);

				// TODO add input switching

				if (i->index > 3)
					return -EINVAL;
				
				dev->vsettings.input = i->index + 1;

				dev_stk11xx_camera_settings(dev);
			}
			break;

		case VIDIOC_QUERYCTRL:
			{
				int i;
				int nbr;
				struct v4l2_queryctrl *c = arg;

				STK_DEBUG("VIDIOC_QUERYCTRL id = %d\n", c->id);

				nbr = sizeof(stk11xx_controls)/sizeof(struct v4l2_queryctrl);

				for (i=0; i<nbr; i++) {
					if (stk11xx_controls[i].id == c->id) {
						STK_DEBUG("VIDIOC_QUERYCTRL found\n");
						memcpy(c, &stk11xx_controls[i], sizeof(struct v4l2_queryctrl));
						switch(c->id)
						{
							case V4L2_CID_BRIGHTNESS:
								c->default_value = dev->vsettings.default_brightness;
								break;
							case V4L2_CID_WHITENESS:
								c->default_value = dev->vsettings.default_whiteness;
								break;
							case V4L2_CID_SATURATION:
								c->default_value = dev->vsettings.default_colour;
								break;
							case V4L2_CID_CONTRAST:
								c->default_value = dev->vsettings.default_contrast;
								break;
							case V4L2_CID_HFLIP:
								c->default_value = dev->vsettings.default_hflip;
								break;
							case V4L2_CID_VFLIP:
								c->default_value = dev->vsettings.default_vflip;
								break;
						}
						break;
					}
				}

				if (i >= nbr)
					return -EINVAL;
			}
			break;

		case VIDIOC_G_CTRL:
			{
				struct v4l2_control *c = arg;

				STK_DEBUG("GET CTRL id=%d\n", c->id);

				switch (c->id) {
					case V4L2_CID_BRIGHTNESS:
						c->value = dev->vsettings.brightness;
						break;

					case V4L2_CID_WHITENESS:
						c->value = dev->vsettings.whiteness;
						break;

					case V4L2_CID_HUE:
						c->value = dev->vsettings.hue;
						break;

					case V4L2_CID_SATURATION:
						c->value = dev->vsettings.colour;
						break;

					case V4L2_CID_CONTRAST:
						c->value = dev->vsettings.contrast;
						break;

					case V4L2_CID_HFLIP:
						c->value = dev->vsettings.hflip;
						break;

					case V4L2_CID_VFLIP:
						c->value = dev->vsettings.vflip;
						break;

					default:
						return -EINVAL;
				}
			}
			break;

		case VIDIOC_S_CTRL:
			{
				struct v4l2_control *c = arg;

				STK_DEBUG("SET CTRL id=%d value=%d\n", c->id, c->value);

				switch (c->id) {
					case V4L2_CID_BRIGHTNESS:
						dev->vsettings.brightness = (0xff00 & c->value);
						break;

					case V4L2_CID_HUE:
						dev->vsettings.hue = (0xff00 & c->value);
						break;

					case V4L2_CID_SATURATION:
						dev->vsettings.colour = (0xff00 & c->value);
						break;

					case V4L2_CID_CONTRAST:
						dev->vsettings.contrast = (0xff00 & c->value);
						break;

					case V4L2_CID_HFLIP:
						dev->vsettings.hflip = c->value ? 1: 0;
						break;

					case V4L2_CID_VFLIP:
						dev->vsettings.vflip = c->value ? 1: 0;
						break;

					default:
						return -EINVAL;
				}

				dev_stk11xx_camera_settings(dev);
			}
			break;

		case VIDIOC_ENUM_FMT:
			{
				int index;
				struct v4l2_fmtdesc *fmtd = arg;

				STK_DEBUG("VIDIOC_ENUM_FMT %d\n", fmtd->index);

				index = fmtd->index;

				memset(fmtd, 0, sizeof(*fmtd));

				fmtd->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				fmtd->index = index;

				switch (index) {
					case 0:
						fmtd->flags = 0;
						fmtd->pixelformat = V4L2_PIX_FMT_RGB24;

						strcpy(fmtd->description, "rgb24");
						break;

					case 1:
						fmtd->flags = 0;
						fmtd->pixelformat = V4L2_PIX_FMT_RGB32;

						strcpy(fmtd->description, "rgb32");
						break;

					case 2:
						fmtd->flags = 0;
						fmtd->pixelformat = V4L2_PIX_FMT_BGR24;

						strcpy(fmtd->description, "bgr24");
						break;

					case 3:
						fmtd->flags = 0;
						fmtd->pixelformat = V4L2_PIX_FMT_BGR32;

						strcpy(fmtd->description, "bgr32");
						break;

					case 4:
						fmtd->flags = 0;
						fmtd->pixelformat = V4L2_PIX_FMT_UYVY;

						strcpy(fmtd->description, "uyvy");
						break;

					case 5:
						fmtd->flags = 0;
						fmtd->pixelformat = V4L2_PIX_FMT_YUYV;

						strcpy(fmtd->description, "yuyv");
						break;

					default:
						return -EINVAL;
				}
			}
			break;

		case VIDIOC_G_FMT:
			{
				struct v4l2_format *fmtd = arg;
				struct v4l2_pix_format pix_format;

				STK_DEBUG("GET FMT %d\n", fmtd->type);

				if (fmtd->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				pix_format.width = dev->view.x;
				pix_format.height = dev->view.y;
				pix_format.field = V4L2_FIELD_NONE;
				pix_format.colorspace = V4L2_COLORSPACE_SRGB;
				pix_format.priv = 0;

				switch (dev->vsettings.palette) {
					case STK11XX_PALETTE_RGB24:
						pix_format.pixelformat = V4L2_PIX_FMT_RGB24;
						pix_format.sizeimage = pix_format.width * pix_format.height * 3;
						pix_format.bytesperline = 3 * pix_format.width;
						break;

					case STK11XX_PALETTE_RGB32:
						pix_format.pixelformat = V4L2_PIX_FMT_RGB32;
						pix_format.sizeimage = pix_format.width * pix_format.height * 4;
						pix_format.bytesperline = 4 * pix_format.width;
						break;

					case STK11XX_PALETTE_BGR24:
						pix_format.pixelformat = V4L2_PIX_FMT_BGR24;
						pix_format.sizeimage = pix_format.width * pix_format.height * 3;
						pix_format.bytesperline = 3 * pix_format.width;
						break;

					case STK11XX_PALETTE_BGR32:
						pix_format.pixelformat = V4L2_PIX_FMT_BGR32;
						pix_format.sizeimage = pix_format.width * pix_format.height * 4;
						pix_format.bytesperline = 4 * pix_format.width;
						break;

					case STK11XX_PALETTE_UYVY:
						pix_format.pixelformat = V4L2_PIX_FMT_UYVY;
						pix_format.sizeimage = pix_format.width * pix_format.height * 2;
						pix_format.bytesperline = 2 * pix_format.width;
						break;

					case STK11XX_PALETTE_YUYV:
						pix_format.pixelformat = V4L2_PIX_FMT_YUYV;
						pix_format.sizeimage = pix_format.width * pix_format.height * 2;
						pix_format.bytesperline = 2 * pix_format.width;
						break;

					default:
						pix_format.pixelformat = 0;
						pix_format.sizeimage = 0;
						pix_format.bytesperline = 0;
				}

				memcpy(&(fmtd->fmt.pix), &pix_format, sizeof(pix_format));
			}
			break;

		case VIDIOC_TRY_FMT:
			{
				struct v4l2_format *fmtd = arg;

				STK_DEBUG("TRY FMT %d\n", fmtd->type);

				if (fmtd->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				switch (dev->webcam_type) {
					case STK11XX_SXGA:
						if (fmtd->fmt.pix.width > stk11xx_image_sizes[STK11XX_1280x1024].x)
							fmtd->fmt.pix.width = stk11xx_image_sizes[STK11XX_1280x1024].x;
						else if (fmtd->fmt.pix.width < stk11xx_image_sizes[0].x)
							fmtd->fmt.pix.width = stk11xx_image_sizes[0].x;
	
						if (fmtd->fmt.pix.height > stk11xx_image_sizes[STK11XX_1280x1024].y)
							fmtd->fmt.pix.height = stk11xx_image_sizes[STK11XX_1280x1024].y;
						else if (fmtd->fmt.pix.height < stk11xx_image_sizes[0].y)
							fmtd->fmt.pix.height = stk11xx_image_sizes[0].y;
						break;

					case STK11XX_PAL:
						if (fmtd->fmt.pix.width > stk11xx_image_sizes[STK11XX_720x576].x)
							fmtd->fmt.pix.width = stk11xx_image_sizes[STK11XX_720x576].x;
						else if (fmtd->fmt.pix.width < stk11xx_image_sizes[0].x)
							fmtd->fmt.pix.width = stk11xx_image_sizes[0].x;
	
						if (fmtd->fmt.pix.height > stk11xx_image_sizes[STK11XX_720x576].y)
							fmtd->fmt.pix.height = stk11xx_image_sizes[STK11XX_720x576].y;
						else if (fmtd->fmt.pix.height < stk11xx_image_sizes[0].y)
							fmtd->fmt.pix.height = stk11xx_image_sizes[0].y;
						break;

					case STK11XX_VGA:
						if (fmtd->fmt.pix.width > stk11xx_image_sizes[STK11XX_640x480].x)
							fmtd->fmt.pix.width = stk11xx_image_sizes[STK11XX_640x480].x;
						else if (fmtd->fmt.pix.width < stk11xx_image_sizes[0].x)
							fmtd->fmt.pix.width = stk11xx_image_sizes[0].x;
	
						if (fmtd->fmt.pix.height > stk11xx_image_sizes[STK11XX_640x480].y)
							fmtd->fmt.pix.height = stk11xx_image_sizes[STK11XX_640x480].y;
						else if (fmtd->fmt.pix.height < stk11xx_image_sizes[0].y)
							fmtd->fmt.pix.height = stk11xx_image_sizes[0].y;
						break;
				}

				fmtd->fmt.pix.field = V4L2_FIELD_NONE;
				fmtd->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
				fmtd->fmt.pix.priv = 0;
				switch (fmtd->fmt.pix.pixelformat) {
					case V4L2_PIX_FMT_RGB24:
					case V4L2_PIX_FMT_BGR24:
						dev->vsettings.depth = 24;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 3;
						fmtd->fmt.pix.bytesperline = 3 * fmtd->fmt.pix.width;
						break;

					case V4L2_PIX_FMT_RGB32:
					case V4L2_PIX_FMT_BGR32:
						dev->vsettings.depth = 32;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 4;
						fmtd->fmt.pix.bytesperline = 4 * fmtd->fmt.pix.width;
						break;

					case V4L2_PIX_FMT_UYVY:
					case V4L2_PIX_FMT_YUYV:
						dev->vsettings.depth = 16;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 2;
						fmtd->fmt.pix.bytesperline = 2 * fmtd->fmt.pix.width;
						break;

					default:
						return -EINVAL;
				}
			}
			break;

		case VIDIOC_S_FMT:
			{
				struct v4l2_format *fmtd = arg;

				STK_DEBUG("SET FMT %d : %d\n", fmtd->type, fmtd->fmt.pix.pixelformat);

				if (fmtd->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				fmtd->fmt.pix.field = V4L2_FIELD_NONE;
				fmtd->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
				fmtd->fmt.pix.priv = 0;

				switch (fmtd->fmt.pix.pixelformat) {
					case V4L2_PIX_FMT_RGB24:
						dev->vsettings.depth = 24;
						dev->vsettings.palette = STK11XX_PALETTE_RGB24;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 3;
						fmtd->fmt.pix.bytesperline = 3 * fmtd->fmt.pix.width;
						break;

					case V4L2_PIX_FMT_RGB32:
						dev->vsettings.depth = 32;
						dev->vsettings.palette = STK11XX_PALETTE_RGB32;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 4;
						fmtd->fmt.pix.bytesperline = 4 * fmtd->fmt.pix.width;
						break;

					case V4L2_PIX_FMT_BGR24:
						dev->vsettings.depth = 24;
						dev->vsettings.palette = STK11XX_PALETTE_BGR24;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 3;
						fmtd->fmt.pix.bytesperline = 3 * fmtd->fmt.pix.width;
						break;

					case V4L2_PIX_FMT_BGR32:
						dev->vsettings.depth = 32;
						dev->vsettings.palette = STK11XX_PALETTE_BGR32;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 4;
						fmtd->fmt.pix.bytesperline = 4 * fmtd->fmt.pix.width;
						break;

					case V4L2_PIX_FMT_UYVY:
						dev->vsettings.depth = 16;
						dev->vsettings.palette = STK11XX_PALETTE_UYVY;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 2;
						fmtd->fmt.pix.bytesperline = 2 * fmtd->fmt.pix.width;
						break;

					case V4L2_PIX_FMT_YUYV:
						dev->vsettings.depth = 16;
						dev->vsettings.palette = STK11XX_PALETTE_YUYV;
						fmtd->fmt.pix.sizeimage = fmtd->fmt.pix.width * fmtd->fmt.pix.height * 2;
						fmtd->fmt.pix.bytesperline = 2 * fmtd->fmt.pix.width;
						break;

					default:
						return -EINVAL;
				}

				STK_DEBUG("Set width=%d, height=%d\n", fmtd->fmt.pix.width, fmtd->fmt.pix.height);

				// Stop the video stream
				dev_stk11xx_stop_stream(dev);
			
				// ISOC and URB cleanup
				usb_stk11xx_isoc_cleanup(dev);

				// Switch off the camera
				dev_stk11xx_camera_off(dev);

				dev_stk11xx_camera_asleep(dev);

				// Select the new video mode
				if (v4l_stk11xx_select_video_mode(dev, fmtd->fmt.pix.width, fmtd->fmt.pix.height)) {
					STK_ERROR("Select video mode failed !\n");
					return -EAGAIN;
				}

				// Clear the buffers
				stk11xx_clear_buffers(dev);

				// Initialize the device
				dev_stk11xx_init_camera(dev);
				dev_stk11xx_camera_on(dev);
				dev_stk11xx_reconf_camera(dev);

				// ISOC and URB init
				usb_stk11xx_isoc_init(dev);

				// Re-start the stream
				dev_stk11xx_start_stream(dev);

				// Video settings
				dev_stk11xx_camera_settings(dev);
			}
			break;

		case VIDIOC_QUERYSTD:
			{
				STK_DEBUG("QUERY STD\n");
				return -EINVAL;
			}
			break;

		case VIDIOC_G_STD:
			{
				v4l2_std_id *std = arg;

				STK_DEBUG("GET STD\n");
		
				*std = V4L2_STD_UNKNOWN;
			}
			break;

		case VIDIOC_S_STD:
			{
				v4l2_std_id *std = arg;

				STK_DEBUG("SET STD\n");
				
				if (*std != V4L2_STD_UNKNOWN)
					return -EINVAL;
			}
			break;

		case VIDIOC_ENUMSTD:
			{
				struct v4l2_standard *std = arg;

				STK_DEBUG("VIDIOC_ENUMSTD\n");

				if (std->index != 0)
					return -EINVAL;

				std->id = V4L2_STD_UNKNOWN;
				strncpy(std->name, "webcam", sizeof(std->name));

				break;
			}

		case VIDIOC_REQBUFS:
			{
				int nbuffers;
				struct v4l2_requestbuffers *rb = arg;

				if (rb->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				if (rb->memory != V4L2_MEMORY_MMAP)
					return -EINVAL;

				nbuffers = rb->count;

				if (nbuffers < 2)
					nbuffers = 2;
				else if (nbuffers > dev->nbuffers)
					nbuffers = dev->nbuffers;

				rb->count = dev->nbuffers;
			}
			break;

		case VIDIOC_QUERYBUF:
			{
				int index;
				struct v4l2_buffer *buf = arg;

				STK_DEBUG("QUERY BUFFERS %d %d\n", buf->index, dev->nbuffers);

				if (buf->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				if (buf->memory != V4L2_MEMORY_MMAP) 
					return -EINVAL;

				index = buf->index;

				if (index < 0 || index >= dev->nbuffers)
					return -EINVAL;

				memset(buf, 0, sizeof(struct v4l2_buffer));

				buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf->index = index;
				buf->m.offset = index * dev->len_per_image;
				buf->bytesused = dev->view_size;
				buf->field = V4L2_FIELD_NONE;
				buf->memory = V4L2_MEMORY_MMAP;
				buf->length = dev->len_per_image;
			}
			break;

		case VIDIOC_QBUF:
			{
				struct v4l2_buffer *buf = arg;

				STK_DEBUG("VIDIOC_QBUF\n");

				if (buf->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				if (buf->memory != V4L2_MEMORY_MMAP)
					return -EINVAL;

				if (buf->index < 0 || buf->index >= dev->nbuffers)
					return -EINVAL;

				buf->flags |= V4L2_BUF_FLAG_QUEUED;
				buf->flags &= ~V4L2_BUF_FLAG_DONE;
			}
			break;

		case VIDIOC_DQBUF:
			{
				int ret;
				struct v4l2_buffer *buf = arg;

				STK_DEBUG("VIDIOC_DQBUF\n");
				
				if (buf->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				add_wait_queue(&dev->wait_frame, &wait);

				while (dev->full_frames == NULL) {
					if (dev->error_status) {
						remove_wait_queue(&dev->wait_frame, &wait);
						set_current_state(TASK_RUNNING);

						return -dev->error_status;
					}

					if (signal_pending(current)) {
						remove_wait_queue(&dev->wait_frame, &wait);
						set_current_state(TASK_RUNNING);

						return -ERESTARTSYS;
					}

					schedule();
					set_current_state(TASK_INTERRUPTIBLE);
				}

				remove_wait_queue(&dev->wait_frame, &wait);
				set_current_state(TASK_RUNNING);

				STK_DEBUG("VIDIOC_DQBUF : frame ready.\n");

				ret = stk11xx_handle_frame(dev);

				if (ret)
					return -EFAULT;

				buf->index = dev->fill_image;
				buf->bytesused = dev->view_size;
				buf->flags = V4L2_BUF_FLAG_MAPPED;
				buf->field = V4L2_FIELD_NONE;
				do_gettimeofday(&buf->timestamp);
				buf->sequence = 0;
				buf->memory = V4L2_MEMORY_MMAP;
				buf->m.offset = dev->fill_image * dev->len_per_image;
				buf->length = dev->len_per_image; //buf->bytesused;

				stk11xx_next_image(dev);
			}
			break;

		case VIDIOC_STREAMON:
			{
				STK_DEBUG("VIDIOC_STREAMON\n");

				usb_stk11xx_isoc_init(dev);
			}
			break;

		case VIDIOC_STREAMOFF:
			{
				STK_DEBUG("VIDIOC_STREAMOFF\n");

				usb_stk11xx_isoc_cleanup(dev);
			}
			break;

		case VIDIOC_G_PARM:
			{
				struct v4l2_streamparm *sp = arg;

				STK_DEBUG("GET PARM %d\n", sp->type);

				if (sp->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
					return -EINVAL;

				sp->parm.capture.capability = 0;
				sp->parm.capture.capturemode = 0;
				sp->parm.capture.timeperframe.numerator = 1;
				sp->parm.capture.timeperframe.denominator = 30;
				sp->parm.capture.readbuffers = 2;
				sp->parm.capture.extendedmode = 0;
			}
			break;


		case VIDIOC_G_AUDIO:
			STK_DEBUG("GET AUDIO\n");
			return -EINVAL;
			break;

		case VIDIOC_S_AUDIO:
			STK_DEBUG("SET AUDIO\n");
			return -EINVAL;
			break;

		case VIDIOC_S_TUNER:
			STK_DEBUG("SET TUNER\n");
			return -EINVAL;
			break;

		case VIDIOC_G_FBUF:
		case VIDIOC_S_FBUF:
		case VIDIOC_OVERLAY:
			return -EINVAL;
			break;

		case VIDIOC_G_TUNER:
		case VIDIOC_G_FREQUENCY:
		case VIDIOC_S_FREQUENCY:
			return -EINVAL;
			break;

		case VIDIOC_QUERYMENU:
			return -EINVAL;
			break;
/*
		case VIDIOC_CROPCAP:
			{
				struct v4l2_cropcap cc;

				cc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				cc.pixelaspect.numerator = 1;
				cc.pixelaspect.denominator = 1;
				cc.bounds.top = 0;
				cc.bounds.left = 0;
				cc.bounds.width = 640;
				cc.bounds.height = 480;
				cc.defrect.top = 0;
				cc.defrect.left = 0;
				cc.defrect.width = 640;
				cc.defrect.height = 480;

				memcpy(arg, &cc, sizeof(cc));
			}
			break;
*/
		default:
			STK_DEBUG("IOCTL unknown !\n");
			return -ENOIOCTLCMD;
	}

	return 0;
}


/** 
 * @param fp File pointer
 * @param cmd Command
 * @param arg Arguements of the command
 * 
 * @returns 0 if all is OK
 *
 * @brief Manage IOCTL
 *
 * This function permits to manage all the IOCTL from the application.
 */
static long v4l_stk11xx_ioctl(struct file *fp,
		unsigned int cmd, unsigned long arg)
{
	long err;
	struct usb_stk11xx *dev;
	struct video_device *vdev;
	
	vdev = video_devdata(fp);
	dev = video_get_drvdata(video_devdata(fp));

	STK_DEBUG("v4l_stk11xx_ioctl %02X\n", (unsigned char) cmd);

	if (dev == NULL)
		return -EFAULT;

	if (vdev == NULL)
		return -EFAULT;

	mutex_lock(&dev->modlock); 

	err = video_usercopy(fp, cmd, arg, v4l_stk11xx_do_ioctl);

	mutex_unlock(&dev->modlock);

	return err;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Register the video device
 *
 * This function permits to register the USB device to the video device.
 */
int v4l_stk11xx_register_video_device(struct usb_stk11xx *dev)
{
	int err;

	err = v4l2_device_register(&dev->interface->dev, &dev->v4l2_dev);
	if (err < 0) {
		STK_ERROR("couldn't register v4l2_device\n");
		kfree(dev);
		return err;
	}

	strcpy(dev->vdev->name, DRIVER_DESC);

//	dev->vdev->parent = &dev->interface->dev;
	dev->vdev->v4l2_dev = &dev->v4l2_dev;
	dev->vdev->fops = &v4l_stk11xx_fops;
	dev->vdev->release = video_device_release;
	dev->vdev->minor = -1;

	video_set_drvdata(dev->vdev, dev);

	err = video_register_device(dev->vdev, VFL_TYPE_GRABBER, -1);

	if (err)
		STK_ERROR("Video register fail !\n");
	else
		STK_INFO("Syntek USB2.0 Camera is now controlling video device /dev/video%d\n", dev->vdev->minor);

	return err;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Unregister the video device
 *
 * This function permits to unregister the video device.
 */
int v4l_stk11xx_unregister_video_device(struct usb_stk11xx *dev)
{
	STK_INFO("Syntek USB2.0 Camera release resources video device /dev/video%d\n", dev->vdev->minor);

	video_set_drvdata(dev->vdev, NULL);
	video_unregister_device(dev->vdev);
	v4l2_device_unregister(&dev->v4l2_dev);

	return 0;
}


/**
 * @var v4l_stk11xx_fops
 *
 * This variable contains some callback
 */
static struct v4l2_file_operations v4l_stk11xx_fops = {
	.owner = THIS_MODULE,
	.open = v4l_stk11xx_open,
	.release = v4l_stk11xx_release,
	.read = v4l_stk11xx_read,
	.poll = v4l_stk11xx_poll,
	.mmap = v4l_stk11xx_mmap,
// patch .ioctl > .unlocked_ioctl
	.unlocked_ioctl = v4l_stk11xx_ioctl,
#if defined(CONFIG_COMPAT) && defined(v4l_compat_ioctl32)
	.compat_ioctl = v4l_compat_ioctl32,
#endif
};

