#pragma once

#include "def.h"
#include <functional>
#include <future>

#include "socket_handle.h"
#include "semaphore.h"

class open_port : boost::noncopyable, public std::enable_shared_from_this<open_port>
{
  int port;
  semaphore exit_lock, ready_lock;
  future<int> accept_thread;
  ip::tcp::acceptor accept_socket;

  // In whitch thread invoke OnNewConnection
  bool sync_call;
  boost::asio::io_service &io;
public:
  open_port(boost::asio::io_service &_io, int port, bool sync_call = false);
  ~open_port();

  typedef function<void(socket_handle, const boost::system::error_code& error)> callback;
  void SetOnNewConnection(callback);

private:
  // By default do nothing
  callback OnNewConnection = [](socket_handle, const boost::system::error_code& error){};

  void StartOtherThread();
  int OtherThread();
};