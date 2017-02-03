/** 
 * @file stk11xx-dev-6a54.c
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


//=============================================================================
//
// STK-6A54 API
//
//=============================================================================


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
int dev_stk6a54_initialize_device(struct usb_stk11xx *dev)
{
	int retok;
	int value;

	STK_INFO("Initialize USB2.0 Syntek Camera\n");

	usb_stk11xx_write_registry(dev, 0x0000, 0x00e0);
	usb_stk11xx_write_registry(dev, 0x0002, 0x00f8);
	usb_stk11xx_write_registry(dev, 0x0002, 0x0078);
	usb_stk11xx_write_registry(dev, 0x0000, 0x0020);

	dev_stk6a54_configure_device(dev, 0);
	retok = dev_stk11xx_check_device(dev, 65);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_read_registry(dev, 0x02ff, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0203, 0x0060);
	usb_stk11xx_write_registry(dev, 0x0208, 0x000a);
	usb_stk11xx_write_registry(dev, 0x0200, 0x0020);
	retok = dev_stk11xx_check_device(dev, 65);
	usb_stk11xx_read_registry(dev, 0x0209, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_read_registry(dev, 0x02ff, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0203, 0x0060);
	usb_stk11xx_write_registry(dev, 0x0208, 0x000b);
	usb_stk11xx_write_registry(dev, 0x0200, 0x0020);
	retok = dev_stk11xx_check_device(dev, 65);
	usb_stk11xx_read_registry(dev, 0x0209, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_read_registry(dev, 0x02ff, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0203, 0x0060);
	usb_stk11xx_write_registry(dev, 0x0208, 0x001c);
	usb_stk11xx_write_registry(dev, 0x0200, 0x0020);
	retok = dev_stk11xx_check_device(dev, 65);
	usb_stk11xx_read_registry(dev, 0x0209, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_read_registry(dev, 0x02ff, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0203, 0x0060);
	usb_stk11xx_write_registry(dev, 0x0208, 0x001d);
	usb_stk11xx_write_registry(dev, 0x0200, 0x0020);
	retok = dev_stk11xx_check_device(dev, 65);
	usb_stk11xx_read_registry(dev, 0x0209, &value);
	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0000, 0x00e0);
	usb_stk11xx_write_registry(dev, 0x0002, 0x00f8);
	usb_stk11xx_write_registry(dev, 0x0002, 0x0078);
	usb_stk11xx_write_registry(dev, 0x0000, 0x0020);
	usb_stk11xx_write_registry(dev, 0x0100, 0x0021);
	usb_stk11xx_read_registry(dev, 0x0000, &value);
	usb_stk11xx_write_registry(dev, 0x0000, 0x004b);


	dev_stk6a54_configure_device(dev, 1);

	usb_stk11xx_set_feature(dev, 0);

	// Device is initialized and is ready !!!
	STK_INFO("Syntek USB2.0 Camera is ready\n");

	return 0;
}


/** 
 * @param dev Device structure
 * @param step The step of configuration [0-11]
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to configure the device.
 *
 * The configuration of device is composed of 12 steps.
 * This function is called by the initialization process.
 *
 * We don't know the meaning of these steps ! We only replay the USB log.
 */
int dev_stk6a54_configure_device(struct usb_stk11xx *dev, int step)
{
	int value;

	//     0,    1
	static const int values_001B[] = {
		0x0e, 0x0e
	};
	static const int values_001C[] = {
		0x46, 0x46
	};
	static const int values_0202[] = {
		0x1e, 0x1e
	};
	static const int values_0110[] = {
		0x00, 0x00
	};
	static const int values_0112[] = {
		0x00, 0x00
	};
	static const int values_0114[] = {
		0x00, 0x00
	};
	static const int values_0115[] = {
		0x00, 0x05
	};
	static const int values_0116[] = {
		0x00, 0xe0
	};
	static const int values_0117[] = {
		0x00, 0x01
	};
	static const int values_0100[] = {
		0x21, 0x21
	};


	STK_DEBUG("dev_stk6a54_configure_device : %d\n", step);

	usb_stk11xx_write_registry(dev, 0x0000, 0x0024);
	usb_stk11xx_write_registry(dev, 0x0002, 0x0078);
	usb_stk11xx_write_registry(dev, 0x0003, 0x0080);
	usb_stk11xx_write_registry(dev, 0x0005, 0x0000);
	
	usb_stk11xx_write_registry(dev, 0x0007, 0x0003);
	usb_stk11xx_write_registry(dev, 0x000d, 0x0000);
	usb_stk11xx_write_registry(dev, 0x000f, 0x0002);
	usb_stk11xx_write_registry(dev, 0x0300, 0x0012);
	usb_stk11xx_write_registry(dev, 0x0350, 0x0041);
	
	usb_stk11xx_write_registry(dev, 0x0351, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0352, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0353, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0018, 0x0010);
	usb_stk11xx_write_registry(dev, 0x0019, 0x0000);
	
	usb_stk11xx_write_registry(dev, 0x001b, values_001B[step]);
	usb_stk11xx_write_registry(dev, 0x001c, values_001C[step]);
	usb_stk11xx_write_registry(dev, 0x0300, 0x0080);
	usb_stk11xx_write_registry(dev, 0x001a, 0x0004);
	usb_stk11xx_write_registry(dev, 0x0202, values_0202[step]);
	
	usb_stk11xx_write_registry(dev, 0x0110, values_0110[step]);
	usb_stk11xx_write_registry(dev, 0x0111, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0112, values_0112[step]);
	usb_stk11xx_write_registry(dev, 0x0113, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0114, values_0114[step]);
	
	usb_stk11xx_write_registry(dev, 0x0115, values_0115[step]);
	usb_stk11xx_write_registry(dev, 0x0116, values_0116[step]);
	usb_stk11xx_write_registry(dev, 0x0117, values_0117[step]);

	usb_stk11xx_read_registry(dev, 0x0100, &value);
	usb_stk11xx_write_registry(dev, 0x0100, values_0100[step]);

	usb_stk11xx_write_registry(dev, 0x02ff, 0x0000); 


	switch (step) {
		case 0:
			usb_stk11xx_write_registry(dev, 0x0203, 0x0060); 

			usb_stk11xx_read_registry(dev, 0x02ff, &value);
			usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);

			usb_stk11xx_write_registry(dev, 0x0203, 0x0060); 

			usb_stk11xx_write_registry(dev, 0x0204, 0x00ff); 
			usb_stk11xx_write_registry(dev, 0x0205, 0x0001); 

			usb_stk11xx_write_registry(dev, 0x0200, 0x0001); 

			break;
	
		case 1:
			usb_stk11xx_write_registry(dev, 0x0203, 0x0060); 

			dev_stk6a54_sensor_settings(dev);

			usb_stk11xx_read_registry(dev, 0x0209, &value);
			usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);

			break;
	}

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
int dev_stk6a54_camera_asleep(struct usb_stk11xx *dev)
{
	int value;

	usb_stk11xx_read_registry(dev, 0x0104, &value);
	usb_stk11xx_read_registry(dev, 0x0105, &value);
	usb_stk11xx_read_registry(dev, 0x0106, &value);

	usb_stk11xx_write_registry(dev, 0x0100, 0x0021);
	usb_stk11xx_write_registry(dev, 0x0116, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0117, 0x0000);
	usb_stk11xx_write_registry(dev, 0x0018, 0x0000);

	usb_stk11xx_read_registry(dev, 0x0000, &value);
	usb_stk11xx_write_registry(dev, 0x0000, 0x0049);

	return 0;
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
int dev_stk6a54_init_camera(struct usb_stk11xx *dev)
{
	return 0;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to set default sensor settings.
 *
 * We set some registers in using a I2C bus.
 * WARNING, the sensor settings can be different following the situation.
 */
int dev_stk6a54_sensor_settings(struct usb_stk11xx *dev)
{
	int i;
	int retok;
	int value;

	int asize;

	static const int values_204[] = {
		0xff, 0x12, 0xff, 0x2c, 0x2e, 0xff, 0x3c, 0x11, 0x09, 0x04, 
		0x13, 0x14, 0x2c, 0x33, 0x3a, 0x3b, 0x3e, 0x43, 0x16, 0x39, 
		0x35, 0x22, 0x37, 0x23, 0x34, 0x36, 0x06, 0x07, 0x0d, 0x0e, 
		0x4c, 0x4a, 0x21, 0x24, 0x25, 0x26, 0x5c, 0x63, 0x46, 0x0c, 
		0x61, 0x62, 0x7c, 0x20, 0x28, 0x6c, 0x6d, 0x6e, 0x70, 0x71, 
		0x73, 0x3d, 0x5a, 0x4f, 0x50, 0xff, 0xe5, 0xf9, 0x41, 0xe0, 
		0x76, 0x33, 0x42, 0x43, 0x4c, 0x87, 0x88, 0xd7, 0xd9, 0xd3, 
		0xc8, 0xc9, 0x7c, 0x7d, 0x7c, 0x7d, 0x7d, 0x7c, 0x7d, 0x7d, 
		0x7d, 0x90, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 
		0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x92, 0x93, 
		0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 0x93, 
		0x93, 0x93, 0x96, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 
		0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0xc3, 0xa4, 0xa8, 0xc5, 
		0xc6, 0xbf, 0xc7, 0xb6, 0xb8, 0xb7, 0xb9, 0xb3, 0xb4, 0xb5, 
		0xb0, 0xb1, 0xb2, 0xc4, 0xc0, 0xc1, 0x86, 0x50, 0x51, 0x52, 
		0x53, 0x54, 0x55, 0x57, 0x5a, 0x5b, 0x5c, 0xc3, 0x7f, 0xda, 
		0xd7, 0xe5, 0xe1, 0xe0, 0xdd, 0x05, 0xff, 0x12, 0x11, 0x17, 
		0x18, 0x19, 0x1a, 0x32, 0x37, 0x4f, 0x50, 0x5a, 0x6d, 0x3d, 
		0x39, 0x35, 0x22, 0x37, 0x23, 0x34, 0x36, 0x06, 0x07, 0x0d, 
		0x0e, 0x4c, 0xff, 0xe0, 0xc0, 0xc1, 0x8c, 0x86, 0x50, 0x51, 
		0x52, 0x53, 0x54, 0x55, 0x5a, 0x5b, 0x5c, 0xd3, 0xe0, 0xff, 
		0xe0, 0xc0, 0xc1, 0x8c, 0x86, 0x50, 0x51, 0x52, 0x53, 0x54, 
		0x55, 0x5a, 0x5b, 0x5c, 0xd3, 0xe0, 0xff, 0xff, 0x04, 0xff, 
		0xff, 0xff, 0x7c, 0x7d, 0x7c, 0x7c, 0x7d, 0x7c, 0x7d, 0x7c, 
		0xff, 0x7c, 0x7d, 0x7c, 0x7d, 0x7c, 0xff, 0x7c, 0x7d, 0x7c, 
		0x7d, 0x7c, 0x7d, 0x7c, 0x7c, 0xff, 0x7c, 0x7d, 0x7c, 0x7d, 
		0x7c, 0x7d, 0xff, 0x92, 0x93, 0xff, 0x90, 0x91, 0x91, 0x91, 
		0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 
		0x91, 0x91, 0x91, 0xff, 0xc3, 0xc7, 0xce, 0xcc, 0xcd, 0xff, 
		0x04, 0xff, 0x04
	};

	static const int values_205[] = {
		0x01, 0x80, 0x00, 0xff, 0xdf, 0x01, 0x32, 0x00, 0x02, 0x28, 
		0xe5, 0x48, 0x0c, 0x78, 0x33, 0xfb, 0x00, 0x11, 0x10, 0x02, 
		0x88, 0x0a, 0x40, 0x00, 0xa0, 0x1a, 0x02, 0xc0, 0xb7, 0x01, 
		0x00, 0x81, 0x99, 0x40, 0x38, 0x82, 0x00, 0x00, 0x3f, 0x3c, 
		0x70, 0x80, 0x05, 0x80, 0x30, 0x00, 0x80, 0x00, 0x02, 0x94, 
		0xc1, 0x34, 0x57, 0xbb, 0x9c, 0x00, 0x7f, 0xc0, 0x24, 0x14, 
		0xff, 0xa0, 0x20, 0x18, 0x00, 0xd0, 0x3f, 0x03, 0x10, 0x82, 
		0x08, 0x80, 0x00, 0x00, 0x03, 0x48, 0x48, 0x08, 0x20, 0x10, 
		0x0e, 0x00, 0x0e, 0x1a, 0x31, 0x5a, 0x69, 0x75, 0x7e, 0x88, 
		0x8f, 0x96, 0xa3, 0xaf, 0xc4, 0xd7, 0xe8, 0x20, 0x00, 0x06, 
		0xe3, 0x05, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x08, 0x19, 0x02, 0x0c, 0x24, 0x30, 0x28, 
		0x26, 0x02, 0x98, 0x80, 0x00, 0x00, 0xed, 0x00, 0x00, 0x11, 
		0x51, 0x80, 0x10, 0x66, 0xa5, 0x64, 0x7c, 0xaf, 0x97, 0xff, 
		0xc5, 0x94, 0x0f, 0x5c, 0xc8, 0x96, 0x3d, 0x00, 0x90, 0x18, 
		0x00, 0x00, 0x88, 0x00, 0x90, 0x18, 0x05, 0xed, 0x00, 0x00, 
		0x01, 0x1f, 0x67, 0x00, 0xff, 0x00, 0x01, 0x40, 0x00, 0x11, 
		0x43, 0x00, 0x4b, 0x09, 0xc0, 0xca, 0xa8, 0x23, 0x00, 0x38, 
		0x12, 0xda, 0x1a, 0xc3, 0x00, 0xc0, 0x1a, 0x88, 0xc0, 0x87, 
		0x41, 0x00, 0x00, 0x04, 0x64, 0x4b, 0x00, 0x1d, 0x00, 0xc8, 
		0x96, 0x00, 0x00, 0x00, 0xc8, 0x96, 0x00, 0x82, 0x00, 0x00, 
		0x04, 0x64, 0x4b, 0x00, 0x3d, 0x00, 0xc8, 0x96, 0x00, 0x00, 
		0x00, 0xa0, 0x78, 0x00, 0x82, 0x00, 0x00, 0x01, 0xfa, 0x01, 
		0x00, 0x00, 0x00, 0x04, 0x0a, 0x09, 0x16, 0x0a, 0x06, 0x09, 
		0x00, 0x00, 0x04, 0x08, 0x28, 0x08, 0x00, 0x00, 0x05, 0x01, 
		0x7f, 0x02, 0x00, 0x0a, 0x05, 0x00, 0x00, 0x07, 0x03, 0x4d, 
		0x04, 0x4d, 0x00, 0x01, 0xc5, 0x00, 0x00, 0x00, 0x23, 0x3c, 
		0x50, 0x63, 0x73, 0x83, 0x92, 0xa0, 0xad, 0xba, 0xc6, 0xd2, 
		0xde, 0xe9, 0xf5, 0x01, 0x0c, 0x00, 0x80, 0x80, 0x80, 0x01, 
		0xfa, 0x01, 0xfa
	};

	asize = ARRAY_SIZE(values_204);

	for(i=0; i<asize; i++) {
		usb_stk11xx_read_registry(dev, 0x02ff, &value);
		usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);

		usb_stk11xx_write_registry(dev, 0x0203, 0x0060);

		usb_stk11xx_write_registry(dev, 0x0204, values_204[i]);
		usb_stk11xx_write_registry(dev, 0x0205, values_205[i]);
		usb_stk11xx_write_registry(dev, 0x0200, 0x0001);

		retok = dev_stk11xx_check_device(dev, 500);

		if (retok != 1) {
			STK_ERROR("Load default sensor settings fail !\n");
			return -1;
		}

		usb_stk11xx_write_registry(dev, 0x02ff, 0x0000);
	}

	usb_stk11xx_read_registry(dev, 0x02ff, &value);

	usb_stk11xx_write_registry(dev, 0x0203, 0x0060);

	usb_stk11xx_write_registry(dev, 0x0208, 0x0004);

	usb_stk11xx_write_registry(dev, 0x0200, 0x0020);

	retok = dev_stk11xx_check_device(dev, 500);

	return 0;
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
int dev_stk6a54_camera_settings(struct usb_stk11xx *dev)
{
	return 0;
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
int dev_stk6a54_set_camera_quality(struct usb_stk11xx *dev)
{
	return 0;
}


/** 
 * @param dev Device structure
 * 
 * @returns 0 if all is OK
 *
 * @brief This function permits to modify the settings of the camera.
 *
 * This functions permits to modify the frame rate per second.
 */
int dev_stk6a54_set_camera_fps(struct usb_stk11xx *dev)
{
	return 0;
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
int dev_stk6a54_start_stream(struct usb_stk11xx *dev)
{
	return 0;
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
int dev_stk6a54_reconf_camera(struct usb_stk11xx *dev)
{
	dev_stk11xx_camera_settings(dev);

	return 0;
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
int dev_stk6a54_stop_stream(struct usb_stk11xx *dev)
{
	return 0;
}


