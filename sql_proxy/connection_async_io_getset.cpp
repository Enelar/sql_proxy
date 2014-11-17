#include "connection_async_io.h"

string connection_async_io::ReadSomething()
{
  // Forcing move semantics
  decltype(read_queue) temp_queue;

  {
    access_lock.Lock();
    if (!read_queue.size())
      return "";
    // Crap code
    std::move(read_queue.begin(), ++read_queue.begin(), std::back_inserter(temp_queue));
  }

  // TODO: Google how to split 1 element from stl container to himself with move semantics
  auto ret = temp_queue.front();
  read_queue.pop_front();
  return ret;
}

void connection_async_io::WriteSomething(const string &data)
{
  access_lock.Lock();
  write_queue.push_back(data);
}