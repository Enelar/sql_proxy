#include "connection_async_io.h"

void connection_async_io::BeginAsyncIO()
{
  access_lock.Lock();
  if (io_sheduled)
    return;

  io_sheduled = true;
  io_thread = async(std::launch::async, &connection_async_io::IOSheduller, this);
  io_thread.wait_for(1ms);
}

void connection_async_io::IOSheduller()
{
  future<void> futures[] =
  {
    async(launch::async, &connection_async_io::ReadFunctor, this),
    async(launch::async, &connection_async_io::WriteFunctor, this)
  };

  // Forcing futures to start
  futures[0].wait_for(1ms);
  futures[1].wait_for(1ms);
}

/*
 * Refactor: functions almost similar.
 */

void connection_async_io::ReadFunctor()
{
  semaphore stack_lock; // need for lambda references
  bool reshedule = true, exit = false;
  static const int read_size = 1400;
  array<unsigned char, read_size> read_buffer;

  auto Condition = [&exit](const boost::system::error_code&, size_t)
  {
    if (exit)
      return false;
    return true;
  };

  auto Complete = [&](const boost::system::error_code&error, size_t size)
  {
    access_lock.Lock();
    reshedule = true;

    if (error)
      __asm int 0x3; // force debuggin
    read_queue.push_back({read_buffer.begin(), read_buffer.begin() + size});
    if (exit)
      stack_lock.TurnOn();
  };

  while (1)
  {
    if (reshedule)
    {
      access_lock.Lock();
      reshedule = false;
      async_read(*handle, read_buffer, Condition, Complete);
    }

    if (disable_io.Status())
      break;

    this_thread::sleep_for(1ms);
  }

  if (reshedule)
    return; // No lambda active
  stack_lock.TurnOn();
  exit = true;
  stack_lock.Lock(); // Waiting for lambda to release
}

void connection_async_io::WriteFunctor()
{
  semaphore stack_lock; // need for lambda references
  bool reshedule = true, exit = false;
  vector<char> write_buffer;

  auto Condition = [&exit](const boost::system::error_code&, size_t)
  {
    if (exit)
      return false;
    return true;
  };

  auto Complete = [&](const boost::system::error_code&error, size_t size)
  {
    access_lock.Lock();
    reshedule = true;

    if (error)
      __asm int 0x3; // force debuggin
    if (exit)
      stack_lock.TurnOn();
  };

  while (1)
  {
    this_thread::sleep_for(1ms);
    if (reshedule)
    {
      decltype(write_queue)::value_type next_message;

      {
        access_lock.Lock();
        if (write_queue.size())
          continue;
        next_message = write_queue.front();
        write_queue.pop_front();
        //copy(write_queue.front().begin()
      }
      reshedule = false;
      async_write(*handle, boost::asio::buffer(&write_buffer[0], write_buffer.size()), Condition, Complete);
    }

    if (disable_io.Status())
      break;
  }

  if (reshedule)
    return; // No lambda active
  stack_lock.TurnOn();
  exit = true;
  stack_lock.Lock(); // Waiting for lambda to release
}