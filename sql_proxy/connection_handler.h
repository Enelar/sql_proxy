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

  void BeginAsyncIO();
private:
  semaphore access_lock;
  bool io_sheduled = false;
  future<void> io_thread;

  list<string> read_queue, write_queue;

  void IOSheduller();
};