/** 
 * @file stk11xx.h
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

#ifndef STK11XX_H
#define STK11XX_H
#include <media/v4l2-device.h>


#define DRIVER_NAME					"stk11xx"					/**< Name of this driver */
#define DRIVER_VERSION				"v2.2.0"					/**< Version of this driver */
#define DRIVER_VERSION_NUM			0x020200					/**< Version numerical of this driver */
#define DRIVER_DESC					"Syntek USB Video Camera"	/**< Short description of this driver */
#define DRIVER_AUTHOR				"Nicolas VIVIEN"			/**< Author of this driver */
#define PREFIX						DRIVER_NAME ": "			/**< Prefix use for the STK "printk" */

#define USB_SYNTEK1_VENDOR_ID		0x174f						/**< Vendor ID of the camera */
#define USB_SYNTEK2_VENDOR_ID		0x05e1						/**< Vendor ID of the camera */

#define USB_STK_A311_PRODUCT_ID		0xa311						/**< Product ID of the camera STK-1125 */
#define USB_STK_A821_PRODUCT_ID		0xa821						/**< Product ID of the camera STK-1135 */
#define USB_STK_AA11_PRODUCT_ID		0xaa11						/**< Product ID of the camera STK-1135 */
#define USB_STK_6A31_PRODUCT_ID		0x6a31						/**< Product ID of the camera DC-NEW */
#define USB_STK_6A33_PRODUCT_ID		0x6a33						/**< Product ID of the camera DC-NEW */
#define USB_STK_6A51_PRODUCT_ID		0x6a51						/**< Product ID of the camera DC-NEW */
#define USB_STK_6A54_PRODUCT_ID		0x6a54						/**< Product ID of the camera DC-NEW */
#define USB_STK_6D51_PRODUCT_ID		0x6d51						/**< Product ID of the camera DC-NEW */

#define USB_STK_0408_PRODUCT_ID		0x0408						/**< Product ID of the camera STK-1160 */
#define USB_STK_0500_PRODUCT_ID		0x0500						/**< Product ID of the camera DC-1125 */
#define USB_STK_0501_PRODUCT_ID		0x0501						/**< Product ID of the camera DC-1125 */


/**
 * @def VID_HARDWARE_STK11XX
 *
 * This value must be inserted into the kernel headers linux/videodev.h
 * It's useful only for the support of V4L v1
 */
#define VID_HARDWARE_STK11XX		88




/**
 * @def MAX_ISO_BUFS
 *   Number maximal of ISOC buffers
 *
 * @def ISO_FRAMES_PER_DESC
 *   Number frames per ISOC descriptor
 *
 * @def ISO_MAX_FRAME_SIZE
 *   Maximale size of frame
 *
 * @def ISO_BUFFER_SIZE
 *   Maximal size of buffer
 */
#define MAX_ISO_BUFS				16
#define ISO_FRAMES_PER_DESC			10
#define ISO_MAX_FRAME_SIZE			3 * 1024
#define ISO_BUFFER_SIZE				(ISO_FRAMES_PER_DESC * ISO_MAX_FRAME_SIZE)


/**
 * @def STK11XX_MAX_IMAGES
 *   Absolute maximum number of buffers available for mmap()
 *
 * @def STK11XX_FRAME_SIZE
 *   Maximum size after decompression. Warning, we use a big buffer to be able to work
 *   with a image resolution 1280x1024 ! If your device doesn't support these resolutions
 *   or that you work in an embedded device, you should use a smaller buffer.
 *   TODO :
 *   Allocate a buffer with a dynamical size adapted for the current resolution...
 */
#define STK11XX_MAX_IMAGES			10
#define STK11XX_FRAME_SIZE			(1280 * 1024 * 4)







/**
 * @def DRIVER_SUPPORT
 * 
 * List of supported device
 *   - DC1125 : USB2.0 Syntek chipset
 */
#define DRIVER_SUPPORT				"Syntek USB Camera : STK-1135"

/**
 * @def CONFIG_STK11XX_DEBUG
 *   Enable / Disable the debug mode.
 *
 * @def STK_INFO(str, args...)
 *   Print information message. 
 *   @a Use this function like the function printf.
 *
 * @def STK_ERROR(str, args...)
 *   Print error message.
 *   @a Use this function like the function printf.
 *
 * @def STK_WARNING(str, args...)
 *   Print warning message.
 *   @a Use this function like the function printf.
 *
 * @def STK_DEBUG(str, args...)
 *   Print debug message.
 *   @a Use this function like the function printf.
 */
#ifndef CONFIG_STK11XX_DEBUG
#define CONFIG_STK11XX_DEBUG 			0
#endif

#if CONFIG_STK11XX_DEBUG

#define STK_INFO(str, args...)			printk(KERN_INFO PREFIX str, ##args)
#define STK_ERROR(str, args...)			printk(KERN_ERR PREFIX str, ##args)
#define STK_WARNING(str, args...)		printk(KERN_WARNING PREFIX str, ##args)
#define STK_DEBUG(str, args...)			printk(KERN_DEBUG PREFIX str, ##args)

#else

#define STK_INFO(str, args...)			printk(KERN_INFO PREFIX str, ##args)
#define STK_ERROR(str, args...)			printk(KERN_ERR PREFIX str, ##args)
#define STK_WARNING(str, args...)		printk(KERN_WARNING PREFIX str, ##args)
#define STK_DEBUG(str, args...)			do { } while(0)

#endif


/**
 * @def CONFIG_STK11XX_DEBUG_STREAM
 *   Enable / Disable the debug mode about the stream.
 *
 * @def STK_STREAM(str, args...)
 *   Print stream debug message. 
 *   @a Use this function like the function printf.
 */
#ifndef CONFIG_STK11XX_DEBUG_STREAM
#define CONFIG_STK11XX_DEBUG_STREAM		0
#endif

#if CONFIG_STK11XX_DEBUG_STREAM

#define STK_STREAM(str, args...)		printk(KERN_DEBUG PREFIX str, ##args)

#else

#define STK_STREAM(str, args...)		do { } while(0)

#endif



/**
 * @enum T_SYNTEK_DEVICE Video camera supported by the driver
 */
typedef enum {
	SYNTEK_STK_M811 = 1,
	SYNTEK_STK_A311 = 2,
	SYNTEK_STK_A821 = 3,
	SYNTEK_STK_6A31 = 4,
	SYNTEK_STK_6A33 = 5,
	SYNTEK_STK_6A51 = 6,
	SYNTEK_STK_6A54 = 7,
	SYNTEK_STK_6D51 = 8,
	SYNTEK_STK_0500 = 9,
	SYNTEK_STK_0408 = 10,
	SYNTEK_STK_AA11 = 11,
} T_SYNTEK_DEVICE;


/**
 * @enum T_STK11XX_VIDEOMODE Video feature supported by camera
 */
typedef enum {
	STK11XX_VGA,						/**< For VGA video camera */
	STK11XX_SXGA,						/**< For SXGA video camera 1.3M */
	STK11XX_PAL,                        /**< For PAL capture card */
	STK11XX_UXGA						/**< For UXGA video camera 2M */
} T_STK11XX_VIDEOMODE;


/** 
 * @enum T_STK11XX_RESOLUTION Video resolution
 */
typedef enum {
	STK11XX_80x60,
	STK11XX_128x96,
	STK11XX_160x120,
	STK11XX_213x160,
	STK11XX_320x240,
	STK11XX_640x480,
	STK11XX_720x576,
	STK11XX_800x600,
	STK11XX_1024x768,
	STK11XX_1280x1024,
	STK11XX_NBR_SIZES
} T_STK11XX_RESOLUTION;


/**
 * @enum T_STK11XX_PALETTE Color palette
 */
typedef enum {
	STK11XX_PALETTE_RGB24,
	STK11XX_PALETTE_RGB32,
	STK11XX_PALETTE_BGR24,
	STK11XX_PALETTE_BGR32,
	STK11XX_PALETTE_UYVY,
	STK11XX_PALETTE_YUYV
} T_STK11XX_PALETTE;


/**
 * @struct stk11xx_iso_buf
 */
struct stk11xx_iso_buf {
	void *data;
	int length;
	int read;
	struct urb *urb;
};


/**
 * @struct stk11xx_frame_buf
 */
struct stk11xx_frame_buf {
	int errors;
	void *data;
	volatile bool odd;
	volatile int filled;
	struct stk11xx_frame_buf *next;
};


/**
 * @struct stk11xx_image_buf
 */
struct stk11xx_image_buf {
	unsigned long offset;				/**< Memory offset */
	int vma_use_count;					/**< VMA counter */
};


/**
 * @struct stk11xx_coord
 */
struct stk11xx_coord {
	int x;								/**< X-coordonate */
	int y;								/**< Y-coordonate */
};


/**
 * @struct stk11xx_video
 */
struct stk11xx_video {
	int fps;							/**< FPS setting */
	int brightness;						/**< Brightness setting */
	int contrast;						/**< Contrast setting */
	int whiteness;						/**< Whiteness setting */
	int colour;							/**< Colour setting */
	int depth;							/**< Depth colour setting */
	int palette;						/**< Palette setting */
	int hue;							/**< Hue setting */
	int hflip;							/**< Horizontal flip */
	int vflip;							/**< Vertical flip */
	int input;                          /**< Input for multiinput cards */
	int norm;							/**< Norm, NTSC or PAL */

	/* Default values for the device. Above are values currently in use. */
	int default_brightness;
	int default_contrast;
	int default_colour;
	int default_whiteness;
	int default_hflip;
	int default_vflip;
};


/**
 * @struct usb_stk11xx
 */
struct usb_stk11xx {
	struct v4l2_device v4l2_dev;
	struct video_device *vdev; 			/**< Pointer on a V4L2 video device */
	struct usb_device *udev;			/**< Pointer on a USB device */
	struct usb_interface *interface;	/**< Pointer on a USB interface */

	int release;						/**< Release of the device (bcdDevice) */
	int webcam_model;					/**< Model of video camera device */
	int webcam_type;					/**< Type of camera : VGA, SXGA (1.3M), UXGA (2M) */

	unsigned char *int_in_buffer;		/**< Interrupt IN buffer */
	size_t int_in_size;					/**< Interrupt IN buffer size */
	__u8 int_in_endpointAddr;			/**< Interrupt IN endpoint address */

	size_t isoc_in_size;				/**< Isochrone IN size */
	__u8 isoc_in_endpointAddr;			/**< Isochrone IN endpoint address */

	int watchdog;						/**< Counter for the software watchdog */

	struct stk11xx_video vsettings;		/**< Video settings (brightness, whiteness...) */

	int error_status;

	int vopen;							/**< Video status (Opened or Closed) */
	int visoc_errors;					/**< Count the number of ISOCH errors */
	int vframes_error;					/**< Count the number of fault frames (so dropped) */
	int vframes_dumped;					/**< Count the number of ignored frames */
	int vsync;							/**< sync on valid frame */
	int v1st_cap;						/**< used to get a clean 1st capture */


	spinlock_t spinlock;				/**< Spin lock */
	struct semaphore mutex;				/**< Mutex */
	wait_queue_head_t wait_frame;		/**< Queue head */
	struct mutex modlock;				/**< To prevent races in video_open(), etc */


	// 1: isoc
	char isoc_init_ok;
	struct stk11xx_iso_buf isobuf[MAX_ISO_BUFS];

	// 2: frame
	int frame_size;
	struct stk11xx_frame_buf *framebuf;
	struct stk11xx_frame_buf *empty_frames, *empty_frames_tail;
	struct stk11xx_frame_buf *full_frames, *full_frames_tail;
	struct stk11xx_frame_buf *fill_frame;
	struct stk11xx_frame_buf *read_frame;

	// 3: image
	int view_size;
	int image_size;
	void *image_data;
	struct stk11xx_image_buf images[STK11XX_MAX_IMAGES];
	int image_used[STK11XX_MAX_IMAGES];
	unsigned int nbuffers;
	unsigned int len_per_image;
	int image_read_pos;
	int fill_image;
	int resolution;
	struct stk11xx_coord view;
	struct stk11xx_coord image;
};


/**
 * @def STK11XX_PERCENT
 *   Calculate a value from a percent
 */
#define STK11XX_PERCENT(x,y) ( ((int)x * (int)y) / 100)


/**
 * @def to_stk11xx_dev(d)
 * Cast a member of a structure out to the containing structure
 */
#define to_stk11xx_dev(d) container_of(d, struct usb_stk11xx, kref)


extern const struct stk11xx_coord stk11xx_image_sizes[STK11XX_NBR_SIZES];

	
int usb_stk11xx_write_registry(struct usb_stk11xx *, __u16, __u16);
int usb_stk11xx_read_registry(struct usb_stk11xx *, __u16, int *);
int usb_stk11xx_set_feature(struct usb_stk11xx *, int);
int usb_stk11xx_set_configuration(struct usb_stk11xx *);
int usb_stk11xx_isoc_init(struct usb_stk11xx *);
void usb_stk11xx_isoc_handler(struct urb *);
void usb_stk11xx_isoc_cleanup(struct usb_stk11xx *);

int dev_stk11xx_decompress(struct usb_stk11xx *);
int dev_stk11xx_initialize_device(struct usb_stk11xx *);
int dev_stk11xx_start_stream(struct usb_stk11xx *);
int dev_stk11xx_stop_stream(struct usb_stk11xx *);
int dev_stk11xx_check_device(struct usb_stk11xx *, int);
int dev_stk11xx_camera_on(struct usb_stk11xx *);
int dev_stk11xx_camera_off(struct usb_stk11xx *);
int dev_stk11xx_camera_asleep(struct usb_stk11xx *);
int dev_stk11xx_init_camera(struct usb_stk11xx *);
int dev_stk11xx_reconf_camera(struct usb_stk11xx *);
int dev_stk11xx_camera_settings(struct usb_stk11xx *);
int dev_stk11xx_set_camera_quality(struct usb_stk11xx *);
int dev_stk11xx_set_camera_fps(struct usb_stk11xx *);
int dev_stk11xx_watchdog_camera(struct usb_stk11xx *);

int v4l_stk11xx_select_video_mode(struct usb_stk11xx *, int, int);
int v4l_stk11xx_register_video_device(struct usb_stk11xx *);
int v4l_stk11xx_unregister_video_device(struct usb_stk11xx *);

int stk11xx_create_sysfs_files(struct video_device *);
void stk11xx_remove_sysfs_files(struct video_device *);

int stk11xx_allocate_buffers(struct usb_stk11xx *);
int stk11xx_reset_buffers(struct usb_stk11xx *);
int stk11xx_clear_buffers(struct usb_stk11xx *);
int stk11xx_free_buffers(struct usb_stk11xx *);
void stk11xx_next_image(struct usb_stk11xx *);
int stk11xx_next_frame(struct usb_stk11xx *);
int stk11xx_handle_frame(struct usb_stk11xx *);

int stk11xx_decompress(struct usb_stk11xx *);


#endif 
