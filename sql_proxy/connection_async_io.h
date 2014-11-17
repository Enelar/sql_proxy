#pragma once

#include "connection_handler.h"

struct connection_async_io : connection_handler
{
  void BeginAsyncIO();
private:
  semaphore disable_io;
  bool io_sheduled = false;
  future<void> io_thread;

  list<string> read_queue, write_queue;

  void IOSheduller();
  void ReadFunctor();
  void WriteFunctor();
};