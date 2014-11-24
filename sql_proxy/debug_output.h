#pragma once

#include "def.h"
#include <iostream>

#include "semaphore.h"

// Crap code. Refactor required
struct dout_type
{
  static semaphore sync;
  bool moved = false;
  ~dout_type();
  dout_type();
  dout_type(const dout_type &) = delete;
  // Forcing cascade move
  dout_type(dout_type &&);
};

struct dout_startup_type
{};

#define dout (dout_static << __FUNCTION__ ":" << __LINE__ << "\t ")
extern dout_startup_type dout_static;

template<typename T>
auto operator<<(dout_startup_type &, const T &)->dout_type;
template<typename T>
auto operator<<(dout_type &, const T &)->dout_type &;

#include "debug_output.hpp"