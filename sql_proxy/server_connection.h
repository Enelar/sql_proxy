#pragma once

#include <map>
#include "connection_async_io.h"

struct server_connection : connection_async_io
{
public:
  typedef function<void(string)> callback;
  void AskSomething(string question, callback);

  void SheduleThread();

  server_connection(boost::asio::io_service &io, const wstring addr, int port);
  ~server_connection();
private:
  void InvokeCallback(int id, const string &answer);
  map<int, string> requests;
  map<int, callback> handlers;
  int cur_task = 0, cur_id = 0;

  void AskingThread();
  string SingleAsk(const string &);
  future<void> asking_thread;
  semaphore exit;
};