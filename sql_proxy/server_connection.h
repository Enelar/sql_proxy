#pragma once

#include <map>
#include "connection_async_io.h"

class server_connection : connection_async_io
{
  map<int, string> answers;
  int cur_task = 0, cur_id = 0;
public:
  void AskSomething(string question, function<void(string)> callback);
};