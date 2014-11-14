#include "open_port.h"

open_port::open_port(boost::asio::io_service &_io, int _port, bool _sync_call)
  : io(_io), port(_port), sync_call(_sync_call), accept_socket(_io)
{
}

open_port::~open_port()
{
  dout << "Closing accept port";
  exit_lock.TurnOff();
  dout << "Wait for accept thread for exit";
  while (accept_thread.wait_for(1ms) != future_status::ready)
    io.run();
  dout << "Port is closed";
}

void open_port::SetOnNewConnection(callback f)
{
  ready_lock.Lock();
  OnNewConnection = f;

  if (exit_lock.Status())
    return;
  exit_lock.TurnOn();
  StartOtherThread();
}

void open_port::StartOtherThread()
{
  auto binded_callback = std::bind(&open_port::OtherThread, shared_from_this());

  accept_thread = async(std::launch::async, binded_callback);
  accept_thread.wait_for(1ms);
}

int open_port::OtherThread()
{
  int status = 0;

  try
  {
    auto new_connection = make_shared<socket_type>(io);

    while (true)
    {
      auto binded_callback = std::bind(OnNewConnection, std::ref(new_connection), std::placeholders::_1);
      accept_socket.async_accept(*new_connection, binded_callback);

    loop:
      std::this_thread::sleep_for(100ms);
      if (!exit_lock.Status())
        break;
      if (ready_lock.Status())
        goto loop;

      ready_lock.TurnOn();
    }
    dout << "Accept thread detect exit";
  }
  catch (...)
  { // Make sure everything catched
    dout << "Accept thread experiences failure";
    status = 0xDEADBAAD;
  }

  accept_socket.close();
  ready_lock.Lock();
  return status;
}
