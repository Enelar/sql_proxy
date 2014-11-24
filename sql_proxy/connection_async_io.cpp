#include "connection_async_io.h"

void connection_async_io::BeginAsyncIO()
{
  auto lock = access_lock.Lock();
  if (io_sheduled)
    return;

  disable_io.TurnOff();
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

void connection_async_io::ReadFunctor()
{
  while (!disable_io.Status())
  {
    auto message = ReceiveCycle();

    if (!message.length())
      continue;

    auto lock = access_lock.Lock();
    read_queue.push_back(message);
  }
}

string connection_async_io::ReceiveCycle()
{
  auto start = chrono::system_clock::now();
  static const int read_size = 1400;
  array<unsigned char, read_size> read_buffer;

  while (!disable_io.Status())
  {
    this_thread::sleep_for(10ms);
    boost::system::error_code er;
    auto available = handle->available(er);

    if (er)
    {
      dout << "Error at available " << er.message();
      disable_io.TurnOn();
    }

    if (!available)
    {
      start = chrono::system_clock::now();
      continue;
    }

    if (chrono::system_clock::now() - start < 5s)
      continue; // for demonstration purpouses

    auto size = handle->receive(boost::asio::buffer(read_buffer));
    return { read_buffer.begin(), read_buffer.begin() + size };
  }

  return "";
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

void connection_async_io::ClearQueues(bool read, bool write)
{
  auto lock = access_lock.Lock();
  read_queue.clear();
  write_queue.clear();
}