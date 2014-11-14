#pragma once

#include "def.h"
#include <functional>

#include "socket.h"
#include "semaphore.h"

class open_port
{
  int port;

public:
  open_port(int port);

  function<void(socketc)> OnNewConnection;
};