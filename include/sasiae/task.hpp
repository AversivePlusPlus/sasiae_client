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

#ifndef TASK_HPP
#define TASK_HPP

#include <stdint.h>

namespace SASIAE {

//! \brief Define the period and priority of a function called by the scheduler
class Task {
protected:
  void (*_func)(void*);
  void* _arg;

  uint32_t _period;

public:
  //! \brief Default constructor
  inline Task(void)
    : _func(0), _arg(0), _period(0) {
  }

  //! \brief Task constructor
  inline Task(void (*func)(void*), void* arg)
    : _func(func), _arg(arg),
      _period(0) {
  }

  //! \brief Task constructor
  template<class Updatable>
  inline Task(Updatable* obj)
    : _func([](void* arg){ ((Updatable*)arg)->update(); }), _arg((void*)obj),
      _period(0) {
  }

  //! \brief Copy Constructor
  inline Task(const Task& other)
    : _func(other._func), _arg(other._arg), _period(other._period) {
  }

  //! \brief Copy operation
  inline Task& operator=(const Task& other) {
    _func = other._func;
    _period = other._period;
    return *this;
  }

  //! \brief Set the interval of time during two task call (in microseconds)
  inline void setPeriod(uint32_t period_us) {
    _period = period_us;
  }

  inline uint32_t period(void) const {
    return _period;
  }

  inline bool operator==(const Task& other) const {
    return _func == other._func;
  }

  inline void operator()(void) const {
    if(_func) {
      _func(_arg);
    }
  }
};

}

#endif//TASK_HPP
