#include "dmclock_server.h"
//#include "dmclock_util.h"
#include <iostream>

namespace dmc = crimson::dmclock;

struct Request {};

int main(int argc, char *argv[])
{
  using ClientId = int;
  using Queue = dmc::PushPriorityQueue<ClientId, Request>;
  using QueueRef = std::unique_ptr<Queue>;
  using dmc::ReqParams;
  std::atomic_bool server_ready {false};

  auto server_ready_f = [&server_ready] () -> bool { return server_ready.load(); };
  ClientId client1 = 117;
  ClientId client2 = 918;
  int c1_count = 0;
  int c2_count = 0;
  dmc::ClientInfo info1(0.0, 1.0, 0.0);
  dmc::ClientInfo info2(0.0, 2.0, 0.0);
  QueueRef pq;

  auto submit_req_f = [&] (const ClientId& c,
                              std::unique_ptr<Request> req,
                              dmc::PhaseType phase,
                              uint64_t req_cost) {
                        if (client1 == c) ++c1_count;
                        else if (client2 == c) ++c2_count;
                        else {
                          std::cout << "got request from neither of two clients";
                          //return 1;
                        }
                      };

      auto client_info_f = [&] (ClientId c) -> const dmc::ClientInfo* {
        if (client1 == c) return &info1;
        else if (client2 == c) return &info2;
        else {
          std::cout << "client info looked up for non-existant client";
          return nullptr;
        }
      };
      pq = QueueRef(new Queue(client_info_f, server_ready_f, submit_req_f));

      ReqParams req_params(1,1);

      auto now = dmc::get_time();

      for (int i = 0; i < 5; ++i) {
        pq->add_request_time(Request{}, client1, req_params, now);
        pq->add_request_time(Request{}, client2, req_params, now);
        now += 0.0001;
      }
      server_ready = true;

      for (int i=0;i<6;i++) pq->request_completed();

      std::cout << "c1_count " << c1_count;
      std::cout << "c2_count"  << c2_count;

      return 0;
}
