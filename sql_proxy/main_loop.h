#pragma once

#include "def.h"
#include <future>
#include "semaphore.h"

struct main_loop
{
  bool Run() const;
  main_loop();
  ~main_loop();

  void Stop();

private:
  future<void> thread;
  volatile bool flag = true;

  void StartThread();
};