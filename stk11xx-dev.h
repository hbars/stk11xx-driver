/** 
 * @file stk11xx-dev.h
 * @author Nicolas VIVIEN
 * @date 2007-11-23
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

#ifndef STK11XX_DEV_H
#define STK11XX_DEV_H

int dev_stk0408_initialize_device(struct usb_stk11xx *);
int dev_stk0408_configure_device(struct usb_stk11xx *, int);
int dev_stk0408_start_stream(struct usb_stk11xx *);
int dev_stk0408_stop_stream(struct usb_stk11xx *);
int dev_stk0408_camera_asleep(struct usb_stk11xx *);
int dev_stk0408_init_camera(struct usb_stk11xx *);
int dev_stk0408_sensor_settings(struct usb_stk11xx *);
int dev_stk0408_reconf_camera(struct usb_stk11xx *);
int dev_stk0408_camera_settings(struct usb_stk11xx *);
int dev_stk0408_set_camera_quality(struct usb_stk11xx *);
int dev_stk0408_set_camera_fps(struct usb_stk11xx *);
int dev_stk0408_decode(struct usb_stk11xx *);

int dev_stka311_initialize_device(struct usb_stk11xx *);
int dev_stka311_configure_device(struct usb_stk11xx *, int);
int dev_stka311_start_stream(struct usb_stk11xx *);
int dev_stka311_stop_stream(struct usb_stk11xx *);
int dev_stka311_camera_asleep(struct usb_stk11xx *);
int dev_stka311_init_camera(struct usb_stk11xx *);
int dev_stka311_sensor_settings(struct usb_stk11xx *);
int dev_stka311_reconf_camera(struct usb_stk11xx *);
int dev_stka311_camera_settings(struct usb_stk11xx *);
int dev_stka311_set_camera_quality(struct usb_stk11xx *);
int dev_stka311_set_camera_fps(struct usb_stk11xx *);

int dev_stka821_initialize_device(struct usb_stk11xx *);
int dev_stka821_configure_device(struct usb_stk11xx *, int);
int dev_stka821_start_stream(struct usb_stk11xx *);
int dev_stka821_stop_stream(struct usb_stk11xx *);
int dev_stka821_camera_asleep(struct usb_stk11xx *);
int dev_stka821_init_camera(struct usb_stk11xx *);
int dev_stka821_sensor_settings(struct usb_stk11xx *);
int dev_stka821_reconf_camera(struct usb_stk11xx *);
int dev_stka821_camera_settings(struct usb_stk11xx *);
int dev_stka821_set_camera_quality(struct usb_stk11xx *);
int dev_stka821_set_camera_fps(struct usb_stk11xx *);

int dev_stk6a31_initialize_device(struct usb_stk11xx *);
int dev_stk6a31_configure_device(struct usb_stk11xx *, int);
int dev_stk6a31_start_stream(struct usb_stk11xx *);
int dev_stk6a31_stop_stream(struct usb_stk11xx *);
int dev_stk6a31_camera_asleep(struct usb_stk11xx *);
int dev_stk6a31_init_camera(struct usb_stk11xx *);
int dev_stk6a31_sensor_settings(struct usb_stk11xx *);
int dev_stk6a31_reconf_camera(struct usb_stk11xx *);
int dev_stk6a31_camera_settings(struct usb_stk11xx *);
int dev_stk6a31_set_camera_quality(struct usb_stk11xx *);
int dev_stk6a31_set_camera_fps(struct usb_stk11xx *);

int dev_stk6a33_initialize_device(struct usb_stk11xx *);
int dev_stk6a33_configure_device(struct usb_stk11xx *, int);
int dev_stk6a33_start_stream(struct usb_stk11xx *);
int dev_stk6a33_stop_stream(struct usb_stk11xx *);
int dev_stk6a33_camera_asleep(struct usb_stk11xx *);
int dev_stk6a33_init_camera(struct usb_stk11xx *);
int dev_stk6a33_sensor_settings(struct usb_stk11xx *);
int dev_stk6a33_reconf_camera(struct usb_stk11xx *);
int dev_stk6a33_camera_settings(struct usb_stk11xx *);
int dev_stk6a33_set_camera_quality(struct usb_stk11xx *);
int dev_stk6a33_set_camera_fps(struct usb_stk11xx *);

int dev_stk6a51_initialize_device(struct usb_stk11xx *);
int dev_stk6a51_configure_device(struct usb_stk11xx *, int);
int dev_stk6a51_start_stream(struct usb_stk11xx *);
int dev_stk6a51_stop_stream(struct usb_stk11xx *);
int dev_stk6a51_camera_asleep(struct usb_stk11xx *);
int dev_stk6a51_init_camera(struct usb_stk11xx *);
int dev_stk6a51_sensor_settings(struct usb_stk11xx *);
int dev_stk6a51_reconf_camera(struct usb_stk11xx *);
int dev_stk6a51_camera_settings(struct usb_stk11xx *);
int dev_stk6a51_set_camera_quality(struct usb_stk11xx *);
int dev_stk6a51_set_camera_fps(struct usb_stk11xx *);

int dev_stk6a54_initialize_device(struct usb_stk11xx *);
int dev_stk6a54_configure_device(struct usb_stk11xx *, int);
int dev_stk6a54_start_stream(struct usb_stk11xx *);
int dev_stk6a54_stop_stream(struct usb_stk11xx *);
int dev_stk6a54_camera_asleep(struct usb_stk11xx *);
int dev_stk6a54_init_camera(struct usb_stk11xx *);
int dev_stk6a54_sensor_settings(struct usb_stk11xx *);
int dev_stk6a54_reconf_camera(struct usb_stk11xx *);
int dev_stk6a54_camera_settings(struct usb_stk11xx *);
int dev_stk6a54_set_camera_quality(struct usb_stk11xx *);
int dev_stk6a54_set_camera_fps(struct usb_stk11xx *);

int dev_stk6d51_initialize_device(struct usb_stk11xx *);
int dev_stk6d51_configure_device(struct usb_stk11xx *, int);
int dev_stk6d51_start_stream(struct usb_stk11xx *);
int dev_stk6d51_stop_stream(struct usb_stk11xx *);
int dev_stk6d51_camera_asleep(struct usb_stk11xx *);
int dev_stk6d51_init_camera(struct usb_stk11xx *);
int dev_stk6d51_sensor_settings(struct usb_stk11xx *);
int dev_stk6d51_reconf_camera(struct usb_stk11xx *);
int dev_stk6d51_camera_settings(struct usb_stk11xx *);
int dev_stk6d51_set_camera_quality(struct usb_stk11xx *);
int dev_stk6d51_set_camera_fps(struct usb_stk11xx *);

int dev_stk0500_initialize_device(struct usb_stk11xx *);
int dev_stk0500_configure_device(struct usb_stk11xx *, int);
int dev_stk0500_start_stream(struct usb_stk11xx *);
int dev_stk0500_stop_stream(struct usb_stk11xx *);
int dev_stk0500_camera_asleep(struct usb_stk11xx *);
int dev_stk0500_init_camera(struct usb_stk11xx *);
int dev_stk0500_sensor_settings(struct usb_stk11xx *);
int dev_stk0500_reconf_camera(struct usb_stk11xx *);
int dev_stk0500_camera_settings(struct usb_stk11xx *);
int dev_stk0500_set_camera_quality(struct usb_stk11xx *);
int dev_stk0500_set_camera_fps(struct usb_stk11xx *);

#endif 
