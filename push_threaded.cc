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
    int req_id;
    bool& req_ready;
    std::mutex& req_mtx;
    std::condition_variable& req_cv;
    Request(int _id, bool& _req_ready, std::mutex& _mtx, std::condition_variable& _cv ) :
	req_id(_id), req_ready(_req_ready), req_mtx(_mtx), req_cv(_cv){};
};

class Scheduler {
    using Queue = dmc::PushPriorityQueue<ClientId, Request>;
    using QueueRef = std::unique_ptr<Queue>;
    using RequestRef = typename Queue::RequestRef;
private:
    Queue queue;
    std::atomic<int> req_ctr {0};
public:

    template <typename ...Args>
    Scheduler(Args&& ...args): queue(std::forward<Args>(args)...) {};

    void add_request(const ClientId& client, const ReqParams& param, const Time& time, Cost cost) {
	bool req_ready = false;
	std::condition_variable req_cv;
	std::mutex req_mtx;
	int req_id = client + req_ctr++;
	Request req(req_id, req_ready, req_mtx, req_cv);
	std::cout << "adding req for req_id:  "<< req_id << std::endl;
	std::cout << "request added!" << std::endl;
	queue.add_request_time(req,client, param, time, cost);
	std::cout << "completing req: " << req_id << std::endl;
	queue.request_completed();
	// All ye of little faith
	if (std::unique_lock<std::mutex> l(req_mtx); !req_ready){
	    std::cout << "Wait initiated!" << std::endl;
	    req_cv.wait(l,[&req_ready]{return req_ready;});
	}

    }
};

void add_request(const ClientId& cid, int req_count, Scheduler &sched) {
    ReqParams param(1,1);
    for(int i=0; i < req_count; i++) {
	auto now = dmc::get_time();
	sched.add_request(cid, param, now, 1);
	std::this_thread::sleep_for(100ms);
    }
}

int main(int argc, char *argv[])
{
    ClientId client1 = 117;
    ClientId client2 = 918;
    //int c1_count = 0;
    //int c2_count = 0;
    dmc::ClientInfo info1(0.0, 1.0, 0.0);
    dmc::ClientInfo info2(0.0, 10.0, 0.0);

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
	std::cout << "notifying...." << req->req_id << std::endl;
	
	{
	    std::lock_guard<std::mutex> lg(req->req_mtx);
	    req->req_ready = true;
	    req->req_cv.notify_one();
	}

	std::cout << "processed_req " << std::endl;
    };
    Scheduler sync_sched {client_info_f, server_ready_f, handle_req_f};
    server_ready = true;

    std::thread th1(add_request, client1, 10, std::ref(sync_sched));
    std::thread th2(add_request, client2, 20, std::ref(sync_sched));
    std::thread th3(add_request, client1, 2, std::ref(sync_sched));
    std::thread th4(add_request, client2, 3, std::ref(sync_sched));
    th1.join();
    th2.join();
    th3.join();
    th4.join();
    return 0;
}
