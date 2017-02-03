/** 
 * @file stk11xx-buf.c
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

#include <linux/usb.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

#include "stk11xx.h"


/** 
 * @var default_nbrframebuf
 *   Number of frame buffer by default
 */
static int default_nbrframebuf = 3;


/** 
 * @param size Size of memory
 * 
 * @returns Address on the allocated memory
 *
 * @brief Allocate a buffer.
 *
 * This function permits to allocate a buffer in memory.
 */
void * stk11xx_rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long addr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);

	if (!mem)
		return NULL;

	memset(mem, 0, size);

	addr = (unsigned long) mem;

	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *) addr));
		addr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	return mem;
}


/** 
 * @param mem Memory address
 * @param size Size of allocated memory
 *
 * @brief Free a buffer
 *
 * This function permits to free a buffer.
 */
void stk11xx_rvfree(void *mem, unsigned long size)
{
	unsigned long addr;

	if (!mem)
		return;

	addr = (unsigned long) mem;

	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *) addr));
		addr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	vfree(mem);
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Allocate all ISOC buffers.
 *
 * This function permits to reserved the memory for each ISOC buffer.
 */
int stk11xx_allocate_buffers(struct usb_stk11xx *dev)
{
	int i;
	void *kbuf;

	STK_DEBUG("Allocate video buffers\n");

	if (dev == NULL)
		return -ENXIO;

	// Allocate isochronous pipe buffers
	for (i=0; i<MAX_ISO_BUFS; i++) {
		if (dev->isobuf[i].data == NULL) {
			kbuf = kzalloc(ISO_BUFFER_SIZE, GFP_KERNEL);

			if (kbuf == NULL) {
				STK_ERROR("Failed to allocate iso buffer %d\n", i);
				return -ENOMEM;
			}

			STK_DEBUG("Allocated iso buffer at %p\n", kbuf);

			dev->isobuf[i].data = kbuf;
		}
	}

	// Allocate frame buffer structure
	if (dev->framebuf == NULL) {
		kbuf = kzalloc(default_nbrframebuf * sizeof(struct stk11xx_frame_buf), GFP_KERNEL);

		if (kbuf == NULL) {
			STK_ERROR("Failed to allocate frame buffer structure\n");
			return -ENOMEM;
		}

		STK_DEBUG("Allocated frame buffer structure at %p\n", kbuf);

		dev->framebuf = kbuf;
	}

	// Create frame buffers and make circular ring
	for (i=0; i<default_nbrframebuf; i++) {
		if (dev->framebuf[i].data == NULL) {
			kbuf = vmalloc(STK11XX_FRAME_SIZE);

			if (kbuf == NULL) {
				STK_ERROR("Failed to allocate frame buffer %d\n", i);
				return -ENOMEM;
			}

			STK_DEBUG("Allocated frame buffer %d at %p.\n", i, kbuf);

			dev->framebuf[i].data = kbuf;
			memset(kbuf, 0, STK11XX_FRAME_SIZE);
		}
	}

	// Allocate image buffer; double buffer for mmap()
	kbuf = stk11xx_rvmalloc(dev->nbuffers * dev->len_per_image);

	if (kbuf == NULL) {
		STK_ERROR("Failed to allocate image buffer(s). needed (%d)\n",
				dev->nbuffers * dev->len_per_image);
		return -ENOMEM;
	}

	STK_DEBUG("Allocated image buffer at %p\n", kbuf);

	dev->image_data = kbuf;

	for (i = 0; i < dev->nbuffers; i++) {
		dev->images[i].offset = i * dev->len_per_image;
		dev->images[i].vma_use_count = 0;
	}

	for (; i < STK11XX_MAX_IMAGES; i++)
		dev->images[i].offset = 0;

	kbuf = NULL;
	
	return 0;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Reset all ISOC buffers.
 *
 * This function permits to reset all ISOC buffers.
 */
int stk11xx_reset_buffers(struct usb_stk11xx *dev)
{
	int i;
	unsigned long flags;

	STK_DEBUG("Reset all buffers\n");

	spin_lock_irqsave(&dev->spinlock, flags);

	dev->full_frames = NULL;
	dev->full_frames_tail = NULL;

	for (i=0; i<dev->nbuffers; i++) {
		dev->framebuf[i].filled = 0;
		dev->framebuf[i].errors = 0;

		if (i > 0)
			dev->framebuf[i].next = &dev->framebuf[i - 1];
		else
			dev->framebuf->next = NULL;
	}

	dev->empty_frames = &dev->framebuf[dev->nbuffers - 1];
	dev->empty_frames_tail = dev->framebuf;
	dev->read_frame = NULL;
	dev->fill_frame = dev->empty_frames;
	dev->empty_frames = dev->empty_frames->next;

	dev->image_read_pos = 0;
	dev->fill_image = 0;

	spin_unlock_irqrestore(&dev->spinlock, flags);

	for (i=0; i<dev->nbuffers; i++)
		dev->image_used[i] = 0;
	
	return 0;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Clear current buffers.
 *
 * This function permits to clear the memory.
 */
int stk11xx_clear_buffers(struct usb_stk11xx *dev)
{
	memset(dev->image_data, 0x00, dev->nbuffers * dev->len_per_image);

	return 0;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Release all buffers.
 *
 * This function permits to release and free the memory for each ISOC buffer.
 */
int stk11xx_free_buffers(struct usb_stk11xx *dev)
{
	int i;

	STK_DEBUG("Free buffers\n");

	if (dev == NULL)
		return -1;

	// Release iso pipe buffers
	for (i=0; i<MAX_ISO_BUFS; i++) {
		if (dev->isobuf[i].data != NULL) {
			kfree(dev->isobuf[i].data);
			dev->isobuf[i].data = NULL;
		}
	}

	// Release frame buffers
	if (dev->framebuf != NULL) {
		for (i=0; i<default_nbrframebuf; i++) {
			if (dev->framebuf[i].data != NULL) {
				vfree(dev->framebuf[i].data);
				dev->framebuf[i].data = NULL;
			}
		}

		kfree(dev->framebuf);
		dev->framebuf = NULL;
	}

	// Release image buffers
	if (dev->image_data != NULL)
		stk11xx_rvfree(dev->image_data, dev->nbuffers * dev->len_per_image);

	dev->image_data = NULL;

	return 0;
}


/** 
 * @param dev Device structure
 *
 * @brief Prepare the next image.
 *
 * This function is called when an image is ready, so as to prepare the next image.
 */
void stk11xx_next_image(struct usb_stk11xx *dev)
{
	STK_STREAM("Select next image\n");

	dev->image_used[dev->fill_image] = 0;
	dev->fill_image = (dev->fill_image + 1) % dev->nbuffers;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Prepare the next frame.
 *
 * This function is called when a frame is ready, so as to prepare the next frame.
 */
int stk11xx_next_frame(struct usb_stk11xx *dev)
{
	int ret = 0;
	unsigned long flags;

	STK_STREAM("Select next frame\n");

	spin_lock_irqsave(&dev->spinlock, flags);

	if (dev->fill_frame != NULL) {
		if (dev->full_frames == NULL) {
			dev->full_frames = dev->fill_frame;
			dev->full_frames_tail = dev->full_frames;
		}
		else {
			dev->full_frames_tail->next = dev->fill_frame;
			dev->full_frames_tail = dev->fill_frame;
		}
	}

	if (dev->empty_frames != NULL) {
		dev->fill_frame = dev->empty_frames;
		dev->empty_frames = dev->empty_frames->next;
	}
	else {
		if (dev->full_frames == NULL) {
			STK_ERROR("Neither empty or full frames available!\n");
			spin_unlock_irqrestore(&dev->spinlock, flags);
			return -EINVAL;
		}

		dev->fill_frame = dev->full_frames;
		dev->full_frames = dev->full_frames->next;

		ret = 1;
	}

	dev->fill_frame->next = NULL;

	spin_unlock_irqrestore(&dev->spinlock, flags);

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Handler frame
 *
 * This function gets called for the isochronous pipe. This function is only called 
 * when a frame is ready. So we have to be fast to decompress the data.
 */
int stk11xx_handle_frame(struct usb_stk11xx *dev)
{
	int ret = 0;
	unsigned long flags;

	STK_STREAM("Sync Handle Frame\n");

	spin_lock_irqsave(&dev->spinlock, flags);

	if (dev->read_frame != NULL) {
		spin_unlock_irqrestore(&dev->spinlock, flags);
		return ret;
	}

	if (dev->full_frames == NULL) {
	}
	else {
		dev->read_frame = dev->full_frames;
		dev->full_frames = dev->full_frames->next;
		dev->read_frame->next = NULL;
	}

	if (dev->read_frame != NULL) {
		spin_unlock_irqrestore(&dev->spinlock, flags);
		ret = dev_stk11xx_decompress(dev);
		spin_lock_irqsave(&dev->spinlock, flags);

		if (dev->empty_frames == NULL) {
			dev->empty_frames = dev->read_frame;
			dev->empty_frames_tail = dev->empty_frames;
		}
		else {
			dev->empty_frames_tail->next = dev->read_frame;
			dev->empty_frames_tail = dev->read_frame;
		}

		dev->read_frame = NULL;
	}

	spin_unlock_irqrestore(&dev->spinlock, flags);

	dev_stk11xx_watchdog_camera(dev);

	return ret;
}

