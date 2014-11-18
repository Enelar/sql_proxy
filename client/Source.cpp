#include <boost\asio.hpp>
#include "../sql_proxy/main_loop.h"

#include <iostream>

using namespace boost::asio::ip;
typedef tcp::socket _socketT;

void program(boost::asio::io_service &io)
{
  cout << "Connecting" << endl;
  {
    tcp::iostream stream;
    stream.connect("127.0.0.1", "10000");
    cout << "Connected" << endl;
    stream << "SEND TEST" << endl;
    cout << "Wait answer" << endl;
    cout << stream.rdbuf();
  }
  cout << "Exiting" << endl;
}

void main()
{
  boost::asio::io_service io;
  program(io);
}