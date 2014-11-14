#pragma once

#include "def.h"
#include <boost/asio.hpp>

using namespace boost::asio;
struct socketc
{
  typedef boost::asio::ip::tcp::socket type;
  type connection;
};
