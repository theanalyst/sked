#include "dmclock_server.h"
#include "dmclock_util.h"
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <atomic>
namespace dmc = crimson::dmclock;
using namespace std::chrono_literals;
using dmc::ReqParams;
using dmc::Time;
using dmc::Cost;
using ClientId = int;
std::mutex log_m;

struct Request {
  bool req_ready = false;
  std::reference_wrapper<std::condition_variable> req_cv;
  Request(std::condition_variable& cv) : req_cv(std::ref(cv)) {};
};

class Scheduler {
  using Queue = dmc::PushPriorityQueue<ClientId, Request>;
  using QueueRef = std::unique_ptr<Queue>;
  //using ClientInfoFunc = std::function<const dmc::ClientInfo*>(ClientId);
  using RequestRef = typename Queue::RequestRef;
private:
  Queue queue;
  //std::atomic<int> ctr{0};
public:

  template <typename ...Args>
  Scheduler(Args&& ...args): queue(std::forward<Args>(args)...) {};

  void add_request(const ClientId& client, const ReqParams& param, const Time& time, Cost cost) {
    std::condition_variable req_cv;
    std::mutex req_mtx;
    Request req(req_cv);
    std::cout << "adding req for client  "<< client << std::endl;
    std::cout << "request added!" << std::endl;
    queue.add_request(req,client, param, time, cost);
    // All ye of little faith
    std::unique_lock<std::mutex> l(req_mtx);
    if (! req.req_ready){
      std::cout << "Wait initiated!" << std::endl;
      req_cv.wait(l,[&req]{return req.req_ready;});
    }
    l.unlock();

    std::cout << "completing req: "<< std::endl;
    queue.request_completed();
  }
};

void add_request(const ClientId& cid, Scheduler &sched) {
  ReqParams param(1,1);
  for(int i=0; i < 10; i++) {
    auto now = dmc::get_time();
    sched.add_request(cid, param, now, 1);
    //std::this_thread::sleep_for(100ms);
  }
}

int main(int argc, char *argv[])
{
  ClientId client1 = 117;
  ClientId client2 = 918;
  //int c1_count = 0;
  //int c2_count = 0;
  dmc::ClientInfo info1(0.0, 1.0, 0.0);
  dmc::ClientInfo info2(0.0, 100.0, 0.0);

  std::atomic_bool server_ready {true};
  auto server_ready_f = [&server_ready] () -> bool { return true; };

  auto client_info_f = [&] (ClientId c) -> const dmc::ClientInfo* {
                         if (client1 == c) return &info1;
                         else if (client2 == c) return &info2;
                         else {
                           std::cout << "client info looked up for non-existant client";
                           return nullptr;
                         }
                       };
  auto handle_req_f = [] (const ClientId& c, std::unique_ptr<Request> req,
                              dmc::PhaseType phase, uint64_t req_cost) {
                        std::cout << "notifying...." << std::endl;
                        {
                          req->req_ready = true;
                          req->req_cv.notify_one();
                        }

                        std::cout << "processed_req " << std::endl;
                      };
  Scheduler sync_sched {client_info_f, server_ready_f, handle_req_f};
  server_ready = true;

  std::thread th1(add_request, client1, std::ref(sync_sched));
  std::thread th2(add_request, client2, std::ref(sync_sched));
  std::thread th3(add_request, client1, std::ref(sync_sched));

  th1.join();
  th2.join();
  th3.join();

  return 0;
}
