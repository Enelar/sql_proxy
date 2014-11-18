#include <boost\lexical_cast.hpp>

#include "connection_handler.h"

using boost::asio::ip::tcp;


void connection_handler::Connect(boost::asio::io_service &io, const wstring addr, int port)
{ // Probably vulnerability
  Connect(io, string{ addr.begin(), addr.end() }, port);
}

void connection_handler::Connect(boost::asio::io_service &io, const string &addr, int port)
{
  access_lock.Lock();
  handle.reset(new _MEMLEAK_ tcp::socket(io));
  tcp::resolver resolver(io);
  connect(*handle, resolver.resolve({ addr, boost::lexical_cast<string>(port) }));
}