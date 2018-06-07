/*
    Aversive++
    Copyright (C) 2014 Eirbot

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SASIAE_MOTOR_HPP
#define SASIAE_MOTOR_HPP

#include <sasiae/sasiae.hpp>
#include <sasiae/client_thread.hpp>
#include <sasiae/device.hpp>

#include <sstream>

#include <stdint.h>

namespace SASIAE {

class Motor : public Device {
public:
  Motor(const char* name)
    : Device(name) {

    ClientThread::instance().
        registerDevice(*this,
                       std::function<void(const char*)>([&] (const char*) mutable -> void {}));
  }

  void put(int32_t val) {
    std::ostringstream oss;

    oss << "value " << ((double)val)/128.;

    ClientThread::instance().
        sendDeviceMessage(*this,
                          oss.str().c_str());
  }
};

}

#endif//SASIAE_MOTOR_HPP
