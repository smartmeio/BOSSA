
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

#ifndef _ANDROIDPORTFACTORY_H
#define _ANDROIDPORTFACTORY_H

#include "SerialPort.h"

#include <sys/types.h>
#include <dirent.h>

#include <string>


class AndroidPortFactory
{
public:
    AndroidPortFactory();
    virtual ~AndroidPortFactory();

    virtual std::string begin();
    virtual std::string end();
    virtual std::string next();

    virtual SerialPort::Ptr create(const std::string& name) {};
    virtual SerialPort::Ptr create(const std::string& name, bool isUsb) {};
    virtual SerialPort::Ptr create(int _fd, bool _isTrick);
private:
    std::string _empty;
};

#endif // _ANDROIDPORTFACTORY_H
