#include "client_connection.h"

void client_connection::Thread()
{
  while (1)
  {
    this_thread::sleep_for(1ms);
    if (exit.Status())
      break;

    auto message = ReadSomething();
    if (!message.length())
      continue;

    sc.AskSomething(message, WriteSomething);
  }
}

void client_connection::SheduleThread()
{
  thread = async(std::launch::async, Thread);
  thread.wait_for(1ms);
}