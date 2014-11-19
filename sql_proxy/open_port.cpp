#include "open_port.h"

open_port::open_port(boost::asio::io_service &_io, int _port, bool _sync_call)
  : io(_io), port(_port), sync_call(_sync_call)
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
  auto lock = ready_lock.Lock();
  OnNewConnection = f;

  if (exit_lock.Status())
    return;
  exit_lock.TurnOn();
  StartOtherThread();
}

void open_port::StartOtherThread()
{
  auto binded_callback = std::bind(&open_port::OtherThread, this);

  accept_thread = async(std::launch::async, binded_callback);
  accept_thread.wait_for(1ms);
}

int open_port::OtherThread()
{
  int status = 0;
  dout << "Accept thread started";

  socket_handle new_connection;
  auto Reshedule = [&](const boost::system::error_code &e)
  {
    OnNewConnection(new_connection, e);
    new_connection = nullptr;
    ready_lock.TurnOn();
  };

  using namespace boost::asio::ip;
  tcp::acceptor accept_socket(io, tcp::endpoint(tcp::v6(), port));
  try
  {

    while (true)
    {
      new_connection = make_shared<socket_type>(io);
      ready_lock.TurnOff();
      accept_socket.async_accept(*new_connection, Reshedule);

    loop:
      std::this_thread::sleep_for(100ms);
      if (!exit_lock.Status())
        break;
      if (!ready_lock.Status())
        goto loop;
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
