
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

#ifndef _LIBUSBSERIALPORT_H
#define _LIBUSBSERIALPORT_H

#include "SerialPort.h"
#include <libusb-1.0/libusb.h>

class LibUsbSerialPort : public SerialPort
{
public:
    LibUsbSerialPort(int termuxFD, bool isTrick);
    virtual ~LibUsbSerialPort();

    bool open(int baud = 115200,
              int data = 8,
              SerialPort::Parity parity = SerialPort::ParityNone,
              SerialPort::StopBit stop = SerialPort::StopBitOne);
    void close();

    bool isUsb() { return _isUsb; };
    bool isTrick() { return _isTrick; };

    int read(uint8_t* data, int size);
    int write(const uint8_t* data, int size);
    int get();
    int put(int c);

    bool timeout(int millisecs);
    void flush();
    void setAutoFlush(bool autoflush);

private:
    int _termuxFd; 
    bool _isUsb;
    bool _isTrick;
    int _timeout;
    bool _autoFlush;
    libusb_device *device;
    libusb_device_handle *handle;
    libusb_context *context;
};

#endif // _LIBUSBSERIALPORT_H
