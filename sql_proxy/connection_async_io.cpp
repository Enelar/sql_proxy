#include "connection_async_io.h"

void connection_async_io::BeginAsyncIO()
{
  auto lock = access_lock.Lock();
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
{ // TODO: Refactor into sync code
  semaphore stack_lock; // need for lambda references
  bool reshedule = true, exit = false;
  static const int read_size = 1400;
  array<unsigned char, read_size> read_buffer;
  boost::asio::streambuf buffer;

  

  auto Complete = [&](const boost::system::error_code&error, size_t size)
  {
    auto lock = access_lock.Lock();
    reshedule = true;

    if (error)
    {
      dout << "Read error " << error.message();
      disable_io.TurnOn();
    }

    auto message = string{ read_buffer.begin(), read_buffer.begin() + size };
    read_queue.push_back(message);
    if (exit)
      stack_lock.TurnOff();
  };

  //typedef decltype() timestamp;
  auto start = chrono::system_clock::now();
  //timestamp start;

  while (1)
  {
    if (reshedule)
    {
      auto lock = access_lock.Lock();
      reshedule = false;
      start = chrono::system_clock::now();

    }

    boost::system::error_code er;
    auto available = handle->available(er);
    if (er)
    {
      dout << "Error at available " << er.message();
      disable_io.TurnOn();
      break;
    }

    if (!available)
      start = chrono::system_clock::now();
    else
    {
      auto test = chrono::system_clock::now() - start;
      //dout << "TEST" << available << " " << start.time_since_epoch() << " " << test.count();
           //dout << test;
      if (chrono::system_clock::now() - start > 5s)
      {
        auto size = handle->receive(boost::asio::buffer(read_buffer));
        Complete(er, size);
      }
    }



    if (disable_io.Status())
      break;

    this_thread::sleep_for(10ms);
  }

  //if (reshedule)
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
//        next_message = "KEEPALIVE";
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