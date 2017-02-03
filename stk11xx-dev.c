/** 
 * @file stk11xx-dev.c
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

#include <linux/usb.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

#include "stk11xx.h"
#include "stk11xx-dev.h"


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to initialize the device.
 *
 * This function must be called at first. It's the start of the
 * initialization process. After this process, the device is
 * completly initalized and it's ready.
 *
 * This function is written from the USB log.
 */
int dev_stk11xx_initialize_device(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_initialize_device(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_initialize_device(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_initialize_device(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_initialize_device(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_initialize_device(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_initialize_device(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_initialize_device(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_initialize_device(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_initialize_device(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function initializes the device for the stream.
 *
 * It's the start. This function has to be called at first, before
 * enabling the video stream.
 */
int dev_stk11xx_init_camera(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_init_camera(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_init_camera(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_init_camera(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_init_camera(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_init_camera(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_init_camera(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_init_camera(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_init_camera(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_init_camera(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * @param nbr Number of tries
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to check the device in reading the register 0x0201.
 *
 * When we configure the stk11xx, this function is used to check the device status.
 *   - If the read value is 0x00, then the device isn't ready.
 *   - If the read value is 0x04, then the device is ready.
 *   - If the read value is other, then the device is misconfigured.
 */
int dev_stk11xx_check_device(struct usb_stk11xx *dev, int nbr)
{
	int i;
	int value;

	for (i=0; i<nbr; i++) {
		usb_stk11xx_read_registry(dev, 0x201, &value);
		
		if (value == 0x00) {
		}
		else if ((value == 0x11) || (value == 0x14)) {
		}
		else if ((value == 0x30) || (value == 0x31)) {
		}
		else if ((value == 0x51)) {
		}
		else if ((value == 0x70) || (value == 0x71)) {
		}
		else if ((value == 0x91)) {
		}
		else if (value == 0x01) {
			return 1;
		}
		else if ((value == 0x04) || (value == 0x05))
			return 1;
		else if (value == 0x15) 
			return 1;
		else {
			STK_ERROR("Check device return error (0x0201 = %02X) !\n", value);
			return -1;
		}
	}

	return 0;
}


/** 
 * @param dev Device structure
 * 
 * @returns Value of register 0x0001
 *
 * @brief A espece of software watchdog.
 *
 * This function reads periodically the value of register 0x0001.
 * 
 * We don't know the purpose. I assume that it seems to a software watchdog.
 */
int dev_stk11xx_watchdog_camera(struct usb_stk11xx *dev)
{
	int value;

	usb_stk11xx_read_registry(dev, 0x0001, &value);

	if (value != 0x03) {
		STK_DEBUG("Error : Register 0x0001 = %02X\n", value);
	}

	return value;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function switchs on the camera.
 *
 * In fact, we choose the alternate interface '5'.
 */
int dev_stk11xx_camera_on(struct usb_stk11xx *dev)
{
	int ret = -1;
	struct usb_device *udev = dev->udev;

	ret = usb_set_interface(udev, 0, 5);

	if (ret < 0)
		STK_ERROR("usb_set_interface failed !\n");

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function switchs off the camera.
 *
 * In fact, we choose the alternate interface '0'.
 */
int dev_stk11xx_camera_off(struct usb_stk11xx *dev)
{
	int ret = -1;
	struct usb_device *udev = dev->udev;

	ret = usb_set_interface(udev, 0, 0);

	if (ret < 0)
		STK_ERROR("usb_set_interface failed !\n");

	return 0;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Wake-up the camera.
 *
 * This function permits to wake-up the device.
 */
int dev_stk11xx_camera_asleep(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_camera_asleep(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_camera_asleep(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_camera_asleep(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_camera_asleep(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_camera_asleep(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_camera_asleep(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_camera_asleep(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_camera_asleep(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_camera_asleep(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to modify the settings of the camera.
 *
 * This functions permits to modify the settings :
 *   - brightness
 *   - contrast
 *   - white balance
 *   - ...
 */
int dev_stk11xx_camera_settings(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_camera_settings(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_camera_settings(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_camera_settings(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_camera_settings(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_camera_settings(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_camera_settings(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_camera_settings(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_camera_settings(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_camera_settings(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to modify the quality video of the camera.
 *
 * This functions permits to modify the settings :
 *   - brightness
 *   - contrast
 *   - white balance
 *   - ...
 */
int dev_stk11xx_set_camera_quality(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_set_camera_quality(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_set_camera_quality(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_set_camera_quality(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_set_camera_quality(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_set_camera_quality(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_set_camera_quality(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_set_camera_quality(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_set_camera_quality(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_set_camera_quality(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to modify the fps of the camera.
 *
 * This functions permits to modify the frame rate per second of the camera.
 * So the number of images per second.
 */
int dev_stk11xx_set_camera_fps(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_set_camera_fps(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_set_camera_fps(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_set_camera_fps(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_set_camera_fps(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_set_camera_fps(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_set_camera_fps(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_set_camera_fps(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_set_camera_fps(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_set_camera_fps(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function sets the device to start the stream.
 *
 * After the initialization of the device and the initialization of the video stream,
 * this function permits to enable the stream.
 */
int dev_stk11xx_start_stream(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_start_stream(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_start_stream(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_start_stream(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_start_stream(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_start_stream(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_start_stream(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_start_stream(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_start_stream(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_start_stream(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Reconfigure the camera before the stream.
 *
 * Before enabling the video stream, you have to reconfigure the device.
 */
int dev_stk11xx_reconf_camera(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_reconf_camera(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_reconf_camera(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_reconf_camera(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_reconf_camera(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_reconf_camera(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_reconf_camera(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_reconf_camera(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_reconf_camera(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_reconf_camera(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function sets the device to stop the stream.
 *
 * You use the function start_stream to enable the video stream. So you
 * have to use the function stop_strem to disable the video stream.
 */
int dev_stk11xx_stop_stream(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			ret = dev_stk0408_stop_stream(dev);
			break;
		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
			ret = dev_stka311_stop_stream(dev);
			break;

		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
			ret = dev_stka821_stop_stream(dev);
			break;

		case SYNTEK_STK_6A31:
			ret = dev_stk6a31_stop_stream(dev);
			break;

		case SYNTEK_STK_6A33:
			ret = dev_stk6a33_stop_stream(dev);
			break;

		case SYNTEK_STK_6A51:
			ret = dev_stk6a51_stop_stream(dev);
			break;

		case SYNTEK_STK_6A54:
			ret = dev_stk6a54_stop_stream(dev);
			break;

		case SYNTEK_STK_6D51:
			ret = dev_stk6d51_stop_stream(dev);
			break;

		case SYNTEK_STK_0500:
			ret = dev_stk0500_stop_stream(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief Decompress/convert a frame from the video stream
 *
 * @param dev Device structure
 */
int dev_stk11xx_decompress(struct usb_stk11xx *dev)
{
	int ret;

	switch (dev->webcam_model) {
		case SYNTEK_STK_0408:
			// Use a particular decompressor based uvyv
			ret = dev_stk0408_decode(dev);
			break;

		case SYNTEK_STK_M811:
		case SYNTEK_STK_A311:
		case SYNTEK_STK_A821:
		case SYNTEK_STK_AA11:
		case SYNTEK_STK_6A31:
		case SYNTEK_STK_6A33:
		case SYNTEK_STK_6A51:
		case SYNTEK_STK_6A54:
		case SYNTEK_STK_6D51:
			// Use a generec decompressor based bayer stream
			ret = stk11xx_decompress(dev);
			break;

		default:
			ret = -1;
	}

	return ret;
}

