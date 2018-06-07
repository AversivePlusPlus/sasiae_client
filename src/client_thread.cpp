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

#include <sasiae/client_thread.hpp>
#include <sasiae/sasiae.hpp>

#include <cstdio>
#include <cstring>
#include <iostream>

#include <unistd.h>

#include <QCommandLineParser>
#include <QCoreApplication>

#include <QHostAddress>
#include <QTcpSocket>

namespace SASIAE {

static QTextStream* _out = NULL;
static QTextStream* _in = NULL;
static QMutex _com_mutex;

static const char* const err_detect_device = "Impossible to detect device's name";
static const char* const err_unknown_device = "Unknown device \"%s\"";
static const char* const err_unknown_command = "Unknown command %c";
static const char* const err_invalid_synchro = "Invalid synchro message";

ClientThread::ClientThread(void) : QThread() {
  _keep_going = true;
  _time = 0;
  _sync_func = NULL;
  _timed_task_done = _synchronized = true;

  if(!_out) {
    QString* str = new QString();
    _out = new QTextStream(str);
  }

  if(!_in) {
    QString* str = new QString();
    _in = new QTextStream(str);
  }

  start();
}

ClientThread::~ClientThread(void) {
}

void ClientThread::quit(void) {
  _keep_going = false;
}

void ClientThread::run(void) {
  while(_keep_going) {
    _buffer = _in->readLine();
    QStringList args = _buffer.split(" ");

    if(args[0] == "T") {
      // Synchronization message
      if(args.length() != 3) {
        sendMessage(ERROR, err_invalid_synchro);
        continue;
      }

      bool ok = true;

      _time = args[1].toULong(&ok);
      if(!ok) {
        sendMessage(ERROR, err_invalid_synchro);
        continue;
      }

      int iteration_num = args[2].toInt(&ok);
      if(!ok) {
        sendMessage(ERROR, err_invalid_synchro);
        continue;
      }

      _timed_task_done = false;
      _synchronized = false;

      // We let the robot loop iterating for some time
      _iteration.release(iteration_num);

      // We execute the scheduler
      if(_sync_func) {
        _sync_func(_time);
      }
      _timed_task_done = true;

      // Let synchronize
      _sync_mutex.lock();
      if(!_synchronized && _timed_task_done && _iteration.available() == 0) {
        sendData("T");
        _synchronized = true;
      }
      _sync_mutex.unlock();
    }
    else if(args[0] == "D") {
      if(args.length() < 2) {
        sendMessage(ERROR, err_detect_device);
        continue;
      }

      const char* name = args[1].toStdString().c_str();

      _devices_mutex.lock();
      if(!_devices.contains(name)) {
        // Unknown device
        sendMessage(ERROR, err_unknown_device);
      }
      else {
        // Device is known, we give its interpreter function the message
        args.removeFirst();
        args.removeFirst();
        _devices[name](args.join(" ").toStdString().c_str());
      }
      _devices_mutex.unlock();
    }
    else if(args[0] == "B") {
      // Begin match message
    }
    else if(args[0] == "S") {
      // Stop message
      _keep_going = false;
      // To unblock the main thread if it's waiting
      _iteration.release();
    }
    else {
      // Unknown command
    }
  }
}

bool ClientThread::sync(void) {
  // Let synchronize if we have finished
  if(_iteration.available() == 0) {
    _sync_mutex.lock();
    if(!_synchronized && _timed_task_done && _iteration.available() == 0) {
      sendData("T");
      _synchronized = true;
    }
    _sync_mutex.unlock();
  }
  _iteration.acquire();
  return _keep_going;
}

bool ClientThread::sendData(const char* data) {
  _com_mutex.lock();
  (*_out) << data << endl;
  _out->flush();
  _com_mutex.unlock();
  return true;
}

bool ClientThread::sendDeviceMessage(const Device& dev, const char* msg) {
  return sendData((std::string("D ") + dev.name() + " " + msg).c_str());
}

bool ClientThread::sendMessage(MessageLevel lvl, const char* msg) {
  char msg_lvl = 'I';
  switch(lvl) {
    case ERROR:
      msg_lvl = 'E';
      break;
    case INFO:
      msg_lvl = 'I';
      break;
    case DEBUG:
      msg_lvl = 'D';
      break;
    case WARNING:
      msg_lvl = 'W';
      break;
  }

  return sendData((std::string("M ") + msg_lvl + " " + msg).c_str());
}

bool ClientThread::registerDevice(const Device& dev, const std::function<void(const char*)> & interpreter) {
  QString device = dev.name();
  _devices_mutex.lock();
  if(_devices.contains(device)) {
    // If the device is already registered
    _devices_mutex.unlock();
    return false;
  }
  else {
    _devices.insert(device, interpreter);
    _devices_mutex.unlock();
    return true;
  }
}

bool ClientThread::setSyncFunction(const std::function<void(int)>& interpreter) {
  _sync_func = interpreter;
  return true;
}

unsigned long int ClientThread::time() const {
  return _time;
}

static void _replace_out(QTextStream* new_out) {
  _com_mutex.lock();
  if(_out) {
    if(_out->string()) {
      (*new_out) << *(_out->string());
      delete(_out->string());
    }
    delete(_out);
  }
  _out = new_out;
  _out->flush();
  _com_mutex.unlock();
}

static void _replace_in(QTextStream* new_in) {
  if(_in) {
    if(_in->string()) {
      delete(_in->string());
    }
    delete(_in);
  }
  _in = new_in;
}

void init(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  QCommandLineParser parser;

  QCommandLineOption addr_option(QStringList() << "s" << "server", "Server address", "address");
  parser.addOption(addr_option);

  QCommandLineOption port_option(QStringList() << "p" << "port", "Server port", "port");
  parser.addOption(port_option);

  parser.parse(app.arguments());

  if(parser.isSet(addr_option)) {
  }
  else {
    _replace_out(new QTextStream(stdout, QIODevice::WriteOnly));
    _replace_in(new QTextStream(stdin, QIODevice::ReadOnly));
  }
}

bool sync(void) {
  return ClientThread::instance().sync();
}

}
