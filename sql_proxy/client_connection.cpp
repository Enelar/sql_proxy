#include "client_connection.h"

void client_connection::Thread()
{
  auto WriteBack = bind(&connection_async_io::WriteSomething, this, std::placeholders::_1);

  while (1)
  {
    this_thread::sleep_for(1ms);
    if (exit.Status())
      break;

    auto message = ReadSomething();
    if (!message.length())
      continue;

    sc.AskSomething(message, WriteBack);
  }
}

void client_connection::SheduleThread()
{
  thread = async(std::launch::async, &client_connection::Thread, this);
  thread.wait_for(1ms);
}

client_connection::client_connection(server_connection &_sc)
  : sc(_sc)
{
}