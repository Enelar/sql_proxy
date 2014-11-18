#include <boost\asio.hpp>
#include "../sql_proxy/main_loop.h"

#include <iostream>

using namespace boost::asio::ip;
typedef tcp::socket _socketT;

void program(boost::asio::io_service &io)
{
  cout << "Wait for connection" << endl;
  {
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v6(), 5432));
    tcp::iostream stream;
    boost::system::error_code ec;
    acceptor.accept(*stream.rdbuf(), ec);
    if (ec)
    {
      cout << "Accept error" << ec.message() << endl;
      return;
    }
    cout << "Accepted" << endl;

    async(std::launch::async, [&stream]()
    {
      while (1)
      {
        if (stream.eof())
          return;
        char ch;
        stream >> ch;
        stream << ch;
        this_thread::sleep_for(1ms); // slowdown for example
      }
      cout << "Echo thread detected eof" << endl;
    });

    while (!stream.eof())
    {
      io.run_one();
      this_thread::sleep_for(1ms);
    }
    cout << "Main thread waiting future" << endl;
  }
  cout << "Exiting" << endl;
}

void main()
{
  boost::asio::io_service io;
  program(io);
}