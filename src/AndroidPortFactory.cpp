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

#include "AndroidPortFactory.h"
#include "LibUsbSerialPort.h"

#include <string.h>
#include <stdio.h>

#include <string>

AndroidPortFactory::AndroidPortFactory()
{
}


AndroidPortFactory::~AndroidPortFactory()
{
}

// SerialPort::Ptr
// AndroidPortFactory::create(const std::string& name)
// {
//     return NULL;
// }

// SerialPort::Ptr
// AndroidPortFactory::create(const std::string& name, bool isUsb)
// {
//     return SerialPort::Ptr(new LibUsbSerialPort(name, isUsb));
// }

SerialPort::Ptr
AndroidPortFactory::create(int _fd, bool _isTrick)
{
    return SerialPort::Ptr(new LibUsbSerialPort(_fd, _isTrick));
}

std::string
AndroidPortFactory::begin()
{
    return next();
}

std::string
AndroidPortFactory::next()
{
    return end();
}

std::string
AndroidPortFactory::end()
{
    return std::string();
}
