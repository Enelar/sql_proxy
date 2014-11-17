#include <map>

#include "open_port.h"
#include "main_loop.h"

#include "client_connection.h"

template<typename T>
int ASMHash(const T *obj)
{
  return reinterpret_cast<int>(obj);
}

void program(boost::asio::io_service &io)
{
  server_connection server;
  server.Connect(io, "127.0.0.1", 5432);

  main_loop loop;
  map<int, unique_ptr<client_connection>> clients;

  auto t = make_shared<open_port>(io, 10000);

  t->SetOnNewConnection([&](socket_handle, const boost::system::error_code& error)
  {
    dout << "On new client connection. ";
    if (error)
    {
      dout << "ERROR: " << error.message();
      loop.Stop();
      return;
    }

    auto client = make_unique<client_connection>(server);
    auto hash = ASMHash<>(client.get());
    clients[hash].reset(client.release());
  });

  while (loop.Run())
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