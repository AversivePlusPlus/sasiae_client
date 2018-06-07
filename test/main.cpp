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

#include <sasiae/device/encoder.hpp>
#include <sasiae/device/motor.hpp>
#include <sasiae/task_manager.hpp>

class MyAsync {
private:
  SASIAE::Motor _motor;
  SASIAE::Task _task;

public:
  MyAsync(void)
    : _task(this), _motor("motor") {
    _task.setPeriod(10);
    SASIAE::TaskManager::instance().addTask(_task);
  }

  void update(void) {
    _motor.put(10);
  }
};

MyAsync as;

int main(int argc, char** argv) {
  SASIAE::init(argc, argv);

  while(SASIAE::sync()) {
  }

  return 0;
}
