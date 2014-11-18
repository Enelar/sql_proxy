#include <map>

#include "open_port.h"
#include "main_loop.h"

#include "client_connection.h"

template<typename T>
int ASMHash(const T *obj)
{
  return reinterpret_cast<int>(obj);
}

pair<int, unique_ptr<client_connection>> ConstructClient(server_connection &s, socket_handle h)
{
  pair<int, unique_ptr<client_connection>> ret;

  ret.second = make_unique<client_connection>(s, h);
  auto p = ret.second.get();
  ret.first = ASMHash<>(p);

  return ret;
}

void program(boost::asio::io_service &io)
{
  server_connection server(io, L"127.0.0.1", 5432);

  main_loop loop;
  map<int, unique_ptr<client_connection>> clients;

  auto t = make_shared<open_port>(io, 2345);

  t->SetOnNewConnection([&](socket_handle h, const boost::system::error_code& error)
  {
    dout << "On new client connection. ";
    if (error)
    {
      dout << "ERROR: " << error.message();
      loop.Stop();
      return;
    }

    clients.insert(ConstructClient(server, h));
  });

  while (loop.Run())
  { // Causing low level io worker not to leave cpu at all
    // (resheduling all the time), causing virtual 100% cpu use.
    io.poll();
    std::this_thread::sleep_for(1ns);
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