#pragma once

#include "def.h"
#include <iostream>

// Crap code. Refactor required
struct dout_type
{
  bool moved = false;
  ~dout_type();
  dout_type() {};
  dout_type(const dout_type &) = delete;
  // Forcing cascade move
  dout_type(dout_type &&);
};

struct dout_startup_type
{};

extern dout_startup_type dout;

template<typename T>
auto operator<<(dout_startup_type &, const T &)->dout_type;
template<typename T>
auto operator<<(dout_type &, const T &)->dout_type &;

#include "debug_output.hpp"