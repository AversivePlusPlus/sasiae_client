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

#include <sasiae/utils/heap.hpp>
#include <sasiae/utils/list.hpp>

#include <sasiae/task.hpp>

#include <sasiae/client_thread.hpp>

namespace SASIAE {

//! \brief System to enable the robot to execute some tasks periodically, using interruptions
template<unsigned int MAX_TASKS>
class TaskManager : public Singleton<TaskManager<MAX_TASKS>> {
  friend class Singleton<TaskManager>;

  //! \brief Default Constructor (implementation dependent)
  TaskManager(void);

  //! \brief Disable interrupts
  void lock(void);

  //! \brief Cancel the effect of lock
  void unlock(void);

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

    //! \brief Return the priority of the task
    uint8_t priority(void) {
      return _priority;
    }
  };

  //! \brief Encapsulate the PrivateTask into an interface usable with the Heap
  class HeapElement {
    PrivateTask* _task;

  public:
    HeapElement(void)
      : _task(0) {
    }

    HeapElement(PrivateTask& tsk)
      : _task(&tsk) {
    }

    bool operator<(const HeapElement& other) {
      return _task->nextCall() > other._task->nextCall()
          || ((_task->nextCall() == other._task->nextCall())
              && (_task->priority() < other._task->priority()));
    }

    PrivateTask& task(void) const { return *_task; }
  };

  Container::Simple::Heap<HeapElement, MAX_TASKS> _heap;
  Container::Simple::List<PrivateTask, MAX_TASKS> _tasks;
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
    while(!_heap.empty() && isInPast(_heap.head().task().nextCall())) {
      HeapElement e = _heap.head();
      e.task().operator ()();
      _heap.pop();

      if(e.task().isUnique()) {
        rmTask(e.task());
      }
      else {
        e.task().setNextCall();
        _heap.push(e);
      }
    }
    unlock();
  }

public:
  //! \brief Add a task to execute
  void addTask(Task& tsk) {
    lock();
    if(!_tasks.full()) {
      _tasks.insert(_tasks.size(), tsk);
      PrivateTask& ptsk = *(_tasks.begin()+(_tasks.size()-1));
      ptsk.setNextCall(_current + ptsk.period());
      TaskManager::instance()._heap.push(HeapElement(ptsk));
    }
    unlock();
  }

  //! \brief Remove a Task
  void rmTask(Task& tsk) {
    lock();
    TaskManager::instance()._heap.flush();
    auto rm = _tasks.end();
    for(auto it = _tasks.begin() ; it != _tasks.end() ; it++) {
      if(tsk == *it) {
        rm = it;
      }
      else {
        TaskManager::instance()._heap.push(HeapElement(*it));
      }
    }
    _tasks.remove(rm - _tasks.begin());
    unlock();
  }

  //! \brief Return the number of slots available
  uint16_t freeSlot(void) const {
    return _tasks.limit() - _tasks.size();
  }
};

struct DefaultSchedulerConfig {
  static constexpr uint32_t MAX_TASKS = 8;
};

template<unsigned int MAX_TASKS>
TaskManager<MAX_TASKS>::TaskManager(void) {
  _current = 0;

  ClientThread::instance().setSyncFunction([&](long long t){
    _current = t;
    processTasks();
  });
}

template<unsigned int MAX_TASKS>
inline void TaskManager<MAX_TASKS>::lock(void) {

}

template<unsigned int MAX_TASKS>
inline void TaskManager<MAX_TASKS>::unlock(void) {

}

}

#endif//SCHEDULER_HPP

