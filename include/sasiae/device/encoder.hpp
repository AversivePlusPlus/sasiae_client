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

#ifndef SASIAE_ENCODER_HPP
#define SASIAE_ENCODER_HPP

#include <sasiae/aversive.hpp>
#include <sasiae/client_thread.hpp>
#include <sasiae/device.hpp>

#include <sstream>
#include <stdint.h>

namespace SASIAE {
class Encoder : public ::SASIAE::Device {
private:
  int32_t _val;

public:
  void get_value_from_msg(int32_t* dest, const char* msg) {
    long long in;
    sscanf(msg, "%lld", &in);
    *dest = (int32_t)in;
  }

  Encoder(const char* name) : Device(name) {
    _val = 0;

    ::SASIAE::ClientThread::instance().
        registerDevice(*this,
                       std::function<void(char*)>([&] (char* msg) mutable -> void {
      using namespace std;
      string cmd;
      long long val;
      istringstream iss(msg);

      iss >> cmd;

      if(cmd == "value") {
        iss >> val;
        _val = val;
      }
      else {
        ::SASIAE::ClientThread::instance().sendMessage(
              ::SASIAE::ClientThread::ERROR,
              "unable to parse message correctly"
              );
      }
    }));

    ::SASIAE::ClientThread::instance().
        sendDeviceMessage(*this, "init");
  }

  int32_t get(void) {
    return _val;
  }

};
}

#endif//SASIAE_ENCODER_HPP
