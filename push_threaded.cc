#include "dmclock_server.h"
#include <iostream>

struct Request {
  int req_id;
  Request (int _id) : req_id (_id) {};
};

class Scheduler {
  using ClientId = int;
  using Queue = dmc::PushPriorityQueue<ClientId, Request>;
  using QueueRef = std::unique_ptr<Queue>;
  using dmc::ReqParams;

private:
  std::atomic_bool server_ready {false};
  auto submit_req_f = [&] (const ClientId& c, std::unique_ptr<Request> req,
                           dmc::PhaseType phase, uint64_t req_cost) {

                      };
  auto server_ready_f = [&server_ready] () -> bool { return server_ready.load(); }

public:
  void set_server_ready() { server_ready = true; };
  void add_request() {};


};
