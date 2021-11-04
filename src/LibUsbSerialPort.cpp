/*
SPDX-license-identifier: Apache-2.0

Copyright (C) 2021 SmartMe.IO

Authors:  Mimmo La Fauci <mimmo@smartme.io>

Licensed under the Apache License, Version 2.0 (the "License"); you may
not use this file except in compliance with the License. You may obtain
a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
License for the specific language governing permissions and limitations
under the License
*/


#include "LibUsbSerialPort.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <errno.h>

#include <string>

#ifndef B460800
#define B460800 460800
#endif
#ifndef B921600
#define B921600 921600
#endif

#define SET_LINE_CODING 0x20        //Configures baud rate, stop-bits, parity, and number-of-character bits.
#define GET_LINE_CODING 0x21        //Requests current DTE rate, stop-bits, parity, and number-of-character bits.
#define SET_CONTROL_LINE_STATE 0x22 //RS232 signal used to tell the DCE device the DTE device is now present
#define SEND_BREAK 0x23

#define SEND_ENCAPSULATED_COMMAND 0x21
#define READ_USB_ENDPOINT 0x81
#define WRITE_USB_ENDPOINT 0x02
/*
 * Output control lines.
 */

#define ACM_CTRL_DTR 0x01
#define ACM_CTRL_RTS 0x02

LibUsbSerialPort::LibUsbSerialPort(int termuxFD, bool isTrick) : 
SerialPort("USB"), _termuxFd(termuxFD), _isUsb(true), _isTrick(isTrick), _timeout(0), _autoFlush(false)
{
}

LibUsbSerialPort::~LibUsbSerialPort()
{
}

bool LibUsbSerialPort::open(int baud,
                            int data,
                            SerialPort::Parity parity,
                            SerialPort::StopBit stop)
{
    struct libusb_device_descriptor desc;
    int result, ret;

    libusb_set_option(NULL, LIBUSB_OPTION_WEAK_AUTHORITY);
    ret = libusb_init(&context);
    if (ret < 0)
    {
        printf("libusb_init failed: %d\n", ret);
        return false;
    }
    ret = libusb_wrap_sys_device(context, (intptr_t)_termuxFd, &handle);
    if (ret < 0)
    {
        printf("libusb_wrap_sys_device failed: %d\n", ret);
        return false;
    }
    else if (handle == NULL)
    {
        printf("libusb_wrap_sys_device returned invalid handle\n");
        return false;
    }
    device = libusb_get_device(handle);
    // get the device descriptor
    libusb_get_device_descriptor(device, &desc);
    printf("USB descr Vendor:%x Product:%x\r\n", desc.idVendor, desc.idProduct);

    /* As we are dealing with a CDC-ACM device, it's highly probable that
        * Linux already attached the cdc-acm driver to this device.
        * We need to detach the drivers from all the USB interfaces. The CDC-ACM
        * Class defines two interfaces: the Control interface and the
        * Data interface.
        */
    for (int if_num = 0; if_num < 2; if_num++)
    {
        if (libusb_kernel_driver_active(handle, if_num))
        {
            libusb_detach_kernel_driver(handle, if_num);
        }
        ret = libusb_claim_interface(handle, if_num);
        if (ret < 0)
        {
            fprintf(stderr, "Error claiming interface %d: %s\n", if_num,
                    libusb_error_name(ret));
        }
    }

    /* Start configuring the device:
        * - set line state
        */
    ret = libusb_control_transfer(handle, SEND_ENCAPSULATED_COMMAND, SET_CONTROL_LINE_STATE, 0,
                                  0, NULL, 0, 5000);
    if (ret < 0)
    {
        fprintf(stderr, "Error during control transfer: %s\n",
                libusb_error_name(ret));
    }

    /* - set line encoding: here 9600 8N1
        * 9600 = 0x2580 ~> 0x80, 0x25 in little endian
        * 115200 = 0x1c200 -> 0x00, 0xc2, 0x01
        */
    unsigned char encoding[] = {0x00, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x08};
    encoding[0] = baud & 0xff;
    encoding[1] = (baud >> 8) & 0xff;
    encoding[2] = (baud >> 16) & 0xff;
    encoding[3] = (baud >> 24) & 0xff;

    ret = libusb_control_transfer(handle, SEND_ENCAPSULATED_COMMAND, SET_LINE_CODING, 0, 0, encoding,
                                  sizeof(encoding), 5000);
    if (ret < 0)
    {
        fprintf(stderr, "Error during control transfer: %s\n",
                libusb_error_name(ret));
    }

    return true;
}

void LibUsbSerialPort::close()
{
    int r = libusb_release_interface(handle, 0); //release the claimed interface
    if (r != 0)
    {
        printf("Cannot Release Interface 0\r\n");
        return;
    }
    r = libusb_release_interface(handle, 1); //release the claimed interface
    if (r != 0)
    {
        //printf("Cannot Release Interface\r\n");
        return;
    }
    libusb_close(handle); //close the device we opened
    libusb_exit(context);
}

int LibUsbSerialPort::read(uint8_t *buffer, int len)
{
    int numread = 0;
    int count = 0;
    int result = 0;
    while (numread < len)
    {
        result = libusb_bulk_transfer(handle, READ_USB_ENDPOINT | LIBUSB_ENDPOINT_IN, buffer, len, &count, _timeout);
        if (result == LIBUSB_ERROR_TIMEOUT)
        {
            return numread;
        }
        else if (result)
        {
            printf("Error reading %d-%d %d\r\n", len, count, result);
        }
        else
        {
            numread += count;
#ifdef _DEBUG_
            printf("Buffer received %d\r\n", count);
            for (int i = 0; i < count; ++i)
                printf("0x%x ", buffer[i]);
            printf("\r\n");
#endif
        }
    }
    if (len != count)
    {
        printf("Reading %d exp %d\r\n", len, count);
    }

    return count;
}

int LibUsbSerialPort::write(const uint8_t *buffer, int len)
{
    int count = 0;
    int result = 0;
    // read the reply
    unsigned timeout = 1000;
    result = libusb_bulk_transfer(handle, WRITE_USB_ENDPOINT, (uint8_t *)buffer, len, &count, timeout);
    if (result)
        printf("Error writing %d-%d\r\n", len, count);
    else
    {
#ifdef _DEBUG_
        printf("Buffer sent %d\r\n", count);
#endif
    }

    return count;
}

int LibUsbSerialPort::get()
{
    uint8_t byte;

    if (read(&byte, 1) != 1)
        return -1;

    return byte;
}

int LibUsbSerialPort::put(int c)
{
    uint8_t byte;

    byte = c;
    return write(&byte, 1);
}

void LibUsbSerialPort::flush()
{
    // There isn't a reliable way to flush on a file descriptor
    // so we just wait it out.  One millisecond is the USB poll
    // interval so that should cover it.
    usleep(1000);
}

bool LibUsbSerialPort::timeout(int millisecs)
{
    _timeout = millisecs;
    return true;
}

void LibUsbSerialPort::setAutoFlush(bool autoflush)
{
    _autoFlush = autoflush;
}
