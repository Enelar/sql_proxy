#include "server_connection.h"

server_connection::server_connection(boost::asio::io_service &io, const wstring addr, int port)
{
  auto AutoReconnect = [](connection_async_io *that)
  { // Probably continiosly memleak, cause constructing lambdas every time
    this_thread::sleep_for(10s);
    if (that->OnDisconnect)
      that->OnDisconnect(*that);
  };

  OnDisconnect = [&, addr, port](connection_async_io &me)
  {
    if (me.AsyncIOActive()) // Wait until future appear. Or we will block thread
    {
      dout << "Server connection: IO still active. Waiting";
      thread(AutoReconnect, &me).detach();
      return;
    }

    dout << "Server connection: Disconnected";
    try
    {
      Connect(io, addr, port);
      dout << "Server connection: Connected";
      BeginAsyncIO();
    }
    catch (boost::system::system_error &e)
    {
      dout << "Auto reconnect failed cause of " << string(e.what());
      thread(AutoReconnect, &me).detach();
    }
  };

  OnDisconnect(*this);
  SheduleThread();
}

void server_connection::AskSomething(string question, function<void(string)> callback)
{
  auto lock = access_lock.Lock(); // TODO: compare and set id iteration
  int askid = ++cur_id; // I dont know is it concurent safe, test required
  dout << "Queue " << askid;

  //access_lock.Lock();
  requests[askid] = question;
  handlers[askid] = callback;
}

void server_connection::InvokeCallback(int id, const string &answer)
{
  // Race is safe http://www.cplusplus.com/reference/map/map/find/
  callback f;

  {
    auto answerto = handlers.find(id);

    if (answerto == handlers.end())
      throw "Request not found";
    f = answerto->second;
  }

  {
    auto lock = access_lock.Lock();
    handlers.erase(id);
  }

  dout << "Answering " << id;
  f(answer);
}

void server_connection::SheduleThread()
{
  asking_thread = async(std::launch::async, &server_connection::AskingThread, this);
  asking_thread.wait_for(1ms);
}

server_connection::~server_connection()
{
  OnDisconnect.swap(decltype(OnDisconnect)());
  exit.TurnOn();
}

void server_connection::AskingThread()
{
  while (1)
  {
    this_thread::sleep_for(1ms);

    if (exit.Status())
      break;
    if (!requests.size())
      continue;

    auto key = requests.begin()->first;
    assert(++cur_task == key);
    auto question = requests[key];

    auto answer = SingleAsk(move(question));
    if (answer.length())
    {
      requests.erase(key);
      InvokeCallback(key, move(answer));
      continue;
    }

    if (exit.Status())
      break;
    if (!AsyncIOActive())
      break;
  }
}

string server_connection::SingleAsk(const string &question)
{
  // We NEED callback at this operation
  WriteSomething(move(question));

  while (true)
  {
    this_thread::sleep_for(1ms);
    if (exit.Status())
      break;
    auto ret = ReadSomething();
    if (ret.length())
      return ret;
    if (!AsyncIOActive())
      break;
  }
  return "";
}