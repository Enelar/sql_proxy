#pragma once

#include "connection_async_io.h"
#include "server_connection.h"

struct client_connection : connection_async_io
{
  future<void> thread;
  semaphore exit;

  void Thread();
public:
  void SheduleThread();

  client_connection(server_connection &, socket_handle sh);
  server_connection &sc;
};