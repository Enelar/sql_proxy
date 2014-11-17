#pragma once

#include <future>
#include <list>

#include "semaphore.h"
#include "socket_handle.h"

struct connection_handler
{
  socket_handle handle;

  void Connect(boost::asio::io_service &io, const wstring addr, int port);
  void Connect(boost::asio::io_service &io, const string &addr, int port);
protected:
  semaphore access_lock;
};