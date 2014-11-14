#pragma once

#include "debug_output.h"

#include <sstream>

template<typename T>
auto operator<<(dout_type &s, const T &obj)->dout_type &
{
  std::wstringstream ss;
  ss << obj;
  return s << ss.str();
}

template<typename T>
auto operator<<(dout_startup_type &, const T &obj)->dout_type
{
  dout_type temporary_object;
  temporary_object << obj;
  return temporary_object;
}

template<>
auto operator<<(dout_type &s, const wstring &obj)->dout_type &;

#include <boost/locale.hpp>
// Workaroud for one codepage
template<>
auto operator<<(dout_type &s, const string &origin)->dout_type &;