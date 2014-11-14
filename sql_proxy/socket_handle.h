#pragma once

#include "def.h"
#include <boost/asio.hpp>
#include <memory>

using namespace boost::asio;
using socket_type = boost::asio::ip::tcp::socket;
using socket_handle = std::shared_ptr<socket_type>;
/*
struct socketc
{
  typedef boost::asio::ip::tcp::socket type;
  type connection;
};
*/
