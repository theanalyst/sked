#include "dmclock_server.h"
#include "dmclock_util.h"
#include <iostream>

namespace dmc = crimson::dmclock;

struct Request {
  int req_id;

  Request(int _id) : req_id(_id) {};
};

void fatal_exit() {
  exit(1);
}

int main(int argc, char *argv[])
{
  using ClientId = int;
  using Queue = dmc::PullPriorityQueue<ClientId, Request>;
  using QueueRef = std::unique_ptr<Queue>;
  using dmc::ReqParams;
  using dmc::PhaseType;
  std::atomic_bool server_ready {false};

  ClientId client1 = 117;
  ClientId client2 = 918;
  int c1_count = 0;
  int c2_count = 0;
  dmc::ClientInfo info1(0.0, 1.0, 0.0);
  dmc::ClientInfo info2(0.0, 100.0, 0.0);
  QueueRef pq;

  auto client_info_f = [&] (ClientId c) -> const dmc::ClientInfo* {
                         if (client1 == c) return &info1;
                         else if (client2 == c) return &info2;
                         else {
                           std::cout << "client info looked up for non-existant client";
                           return nullptr;
                         }
                       };
  pq = QueueRef(new Queue(client_info_f, false));

  ReqParams req_params(1,1);

  auto now = dmc::get_time();

  for (int i = 0; i < 5; ++i) {
    pq->add_request(Request{client1+i}, client1, req_params, now);
    pq->add_request(Request{client2+i}, client2, req_params, now);
    now += 0.0001;
  }

  for (int i=0;i<6;i++) {
    Queue::PullReq pr = pq->pull_request();
    if (Queue::NextReqType::returning != pr.type){
      std::cout << "queue is not returning!" << std::endl;
      fatal_exit();
    }
    auto& retn = boost::get<Queue::PullReq::Retn>(pr.data);

    if (client1 == retn.client) ++c1_count;
    else if (client2 == retn.client) ++c2_count;
    else std::cout << "got request from neither of two clients";

    if (PhaseType::priority != retn.phase)
      std::cout << "invalid phase type returned" << std::endl;

    std::cout << "processed request req=" << retn.request->req_id << std::endl;
  }

  std::cout << "c1_count " << c1_count;
  std::cout << "c2_count"  << c2_count;

      return 0;
}
