#include "debug_output.h"

dout_startup_type dout_static;
semaphore dout_type::sync;

#include <windows.h>

namespace
{
  struct A
  {
    A()
    { // Fix russian characters in russian console with russian way
      setlocale(0, "");
    }
  } crappy_hacky_lammy_shame_thing;
};

template<>
auto operator<<(dout_type &s, const wstring &obj)->dout_type &
{
  // Could attach at any time
  if (IsDebuggerPresent())
    OutputDebugStringW(obj.c_str());
  wcout << obj;

  return s;
}

template<>
auto operator<<(dout_type &s, const string &obj)->dout_type &
{
  if (IsDebuggerPresent())
    OutputDebugStringA(obj.c_str());
  cout << obj;
  return s;
}

dout_type::~dout_type()
{
  if (moved)
    return;
  (*this) << '\n';
  sync.TurnOff();
}

#include <future>

dout_type::dout_type()
{
  // Wait for single lock to occure
  while (!sync.Move())
    this_thread::sleep_for(1ms);
}

dout_type::dout_type(dout_type &&o)
{
  o.moved = true;
}
