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

#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <sasiae/utils/singleton.hpp>
#include <sasiae/task.hpp>
#include <sasiae/client_thread.hpp>

namespace SASIAE {

//! \brief System to enable the robot to execute some tasks periodically, using interruptions
class TaskManager : public Singleton<TaskManager> {
  friend class Singleton<TaskManager>;

  //! \brief Default Constructor (implementation dependent)
  TaskManager(void) {
    _current = 0;

    ClientThread::instance().setSyncFunction([&](long long t){
      _current = t;
      processTasks();
    });
  }

  //! \brief Disable interrupts
  void lock(void) {}

  //! \brief Cancel the effect of lock
  void unlock(void) {}

  //! \brief A task with more information used by the scheduler
  class PrivateTask : public Task {
  private:
    uint32_t _next_call;
  public:
    //! \brief Default constructor
    PrivateTask(void)
      : Task(), _next_call(0) {
    }

    //! \brief Copy constructor
    PrivateTask(const PrivateTask& other)
      : Task((Task&)other), _next_call(other._next_call) {
    }

    //! \brief Task constructor
    PrivateTask(const Task& other)
      : Task(other), _next_call(0) {
    }

    //! \brief Copy operator
    PrivateTask& operator=(const PrivateTask& other) {
      (Task&)(*this) = (const Task&)other;
      _next_call = other._next_call;
      return *this;
    }

    //! \brief Copy Task operator
    PrivateTask& operator=(const Task& other) {
      (Task&)(*this) = other;
      _next_call = 0;
    }

    //! \brief Get the next call date
    uint32_t nextCall(void) const {
      return _next_call;
    }

    //! \brief Add the task's period to the nextCall date
    void setNextCall(void) {
      _next_call += _period;
    }

    //! \brief Add a value to the next call date
    void setNextCall(uint32_t val) {
      _next_call += val;
    }
  };

  QList<PrivateTask> _tasks;
  uint32_t _current;

  //! \brief Check if a date is anterior to the current date
  inline bool isInPast(uint32_t date) {
    constexpr uint32_t LIM = (1L << 31);
    constexpr uint32_t HIG = LIM + LIM / 2;
    constexpr uint32_t LOW = LIM - LIM / 2;

    if(_current < LOW && date > HIG) {
      return true;
    }
    else if(date < LOW && _current > HIG) {
      return false;
    }
    else {
      return date < _current;
    }
  }

  //! \brief Execute current tasks
  inline void processTasks(void) {
    lock();
    for(QList<PrivateTask>::Iterator it = _tasks.begin() ; it != _tasks.end() ; it++) {
      while(it->period() != 0 && isInPast(it->nextCall())) {
        (*it)();
        it->setNextCall();
      }
    }
    unlock();
  }

public:
  //! \brief Add a task to execute
  void addTask(Task& tsk) {
    lock();
    _tasks.insert(_tasks.size(), tsk);
    PrivateTask& ptsk = *(_tasks.begin()+(_tasks.size()-1));
    ptsk.setNextCall(_current + ptsk.period());
    unlock();
  }
};

}

#endif//SCHEDULER_HPP

