#include "main_loop.h"

main_loop::main_loop()
{
  StartThread();
}

main_loop::~main_loop()
{
  if (flag)
  {
    dout << "[!!!] Main loop waiting for user action to shutdown listen thread";
    cout << "Please press ENTER";
  }
  while (thread.wait_for(1ms) != future_status::ready)
    this_thread::sleep_for(1ms);
}

bool main_loop::Run() const
{
  return flag;
}

void main_loop::Stop()
{
  flag = false;
}

void main_loop::StartThread()
{
  dout << "Main loop thread started";
  auto listen = [this]()
  {
    getc(stdin); // Whoa what the hack!
    dout << "Main loop exiting";
    Stop();
  };

  thread = async(std::launch::async, listen);
  thread.wait_for(1ms);
}