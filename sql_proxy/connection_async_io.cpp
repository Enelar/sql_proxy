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
  dout << "Connection async io started";
  // TODO: Catch exceptions
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

  dout << "Connection async io finished";
  io_sheduled = false;
  if (OnDisconnect)
    OnDisconnect(*this);
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
  boost::asio::streambuf buffer;

  auto Complete = [&](const boost::system::error_code&error, size_t size)
  {
    access_lock.Lock();
    reshedule = true;

    if (error)
    {
      dout << "Read error " << error.message();
      disable_io.TurnOn();
    }

    //read_queue.push_back({read_buffer.begin(), read_buffer.begin() + size});
    if (exit)
      stack_lock.TurnOff();
  };

  while (1)
  {
    if (reshedule)
    {
      access_lock.Lock();
      reshedule = false;
      async_read(*handle, boost::asio::buffer(read_buffer), Complete);
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
  boost::system::error_code error;

  while (1)
  {
    this_thread::sleep_for(1ms);
    decltype(write_queue)::value_type next_message;

    {
      access_lock.Lock();
      if (write_queue.size())
      {
        next_message = write_queue.front();
        write_queue.pop_front();
      }
      else
      {
        this_thread::sleep_for(100ms);
        next_message = "KEEPALIVE";
      }
    }

    write(*handle, boost::asio::buffer(next_message), error);

    if (error)
    {
      dout << "Writing failed " << error.message();
      disable_io.TurnOn();
    }

    if (disable_io.Status())
      break;
  }
}

bool connection_async_io::AsyncIOActive() const
{
  return io_sheduled;
}