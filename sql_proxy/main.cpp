#include "open_port.h"

void program(boost::asio::io_service &io)
{
  auto t = make_shared<open_port>(io, 10000);
  bool work = true;

  t->SetOnNewConnection([&work](socket_handle, const boost::system::error_code& error)
  {
    dout << "On new client connection. ";
    if (error)
    {
      dout << "ERROR: " << error.message();
      work = false;
      return;
    }
  });

  while (work)
  {
    io.run();
    std::this_thread::sleep_for(1ms);
  }
  dout << "Main cycle exit";

  std::this_thread::sleep_for(100ms);
}

void main()
{
  dout << "==Program startup";
  boost::asio::io_service io;
  program(io);
  dout << "==Program exit";
}