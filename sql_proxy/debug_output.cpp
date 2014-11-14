#include "debug_output.h"

dout_startup_type dout;

#include <windows.h>

template<>
auto operator<<(dout_type &s, const wstring &obj)->dout_type &
{
  // Could attach at any time
  if (IsDebuggerPresent())
    OutputDebugString(obj.c_str());

  return s;
}

dout_type::~dout_type()
{
  if (!moved)
    (*this) << '\n';
}

dout_type::dout_type(dout_type &&o)
{
  o.moved = true;
}
