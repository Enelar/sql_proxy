#include "server_connection.h"

server_connection::server_connection(boost::asio::io_service &io, const wstring addr, int port)
{
  OnDisconnect = [&](connection_async_io &)
  {
    Connect(io, addr, port);
  };
  Connect(io, addr, port);
}

void server_connection::AskSomething(string question, function<void(string)> callback)
{
  access_lock.Lock(); // TODO: compare and set id iteration
  int askid = ++cur_id; // I dont know is it concurent safe, test required
  dout << "Queue " + askid;

  access_lock.Lock();
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
    access_lock.Lock();
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
    requests.erase(key);

    auto answer = SingleAsk(move(question));
    if (!answer.length())
      if (exit.Status())
        break;
    InvokeCallback(key, move(answer));
  }
}

string server_connection::SingleAsk(const string &question)
{
  // We NEED callback at this operatino
  WriteSomething(move(question));

  while (true)
  {
    this_thread::sleep_for(1ms);
    if (exit.Status())
      break;
    auto ret = ReadSomething();
    if (ret.length())
      return ret;
  }
  return "";
}