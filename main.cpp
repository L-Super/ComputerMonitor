#include <iostream>
#include <zmq.hpp>
#include <thread>
#include "Monitor.h"
#include "spdlog/spdlog.h"

int main()
{
	using namespace std::chrono_literals;

//	int i = 0, j = 0, k = 0;
//	std::tie(i, j, k) = zmq::version();
	auto [i, j, k] = zmq::version();//结构化绑定
	std::array<int, 3> version{ i, j, k };
	spdlog::info("RAID monitor server launched. ZMQ version is {}", spdlog::fmt_lib::join(version, "."));

	// initialize the zmq context with a single IO thread
	zmq::context_t context{ 1 };

	// construct a REP (reply) socket and bind to interface
	zmq::socket_t socket{ context, zmq::socket_type::rep };
	socket.bind("tcp://*:20777");

	assert(socket.handle() != nullptr);
	spdlog::info("bind to port 20777");

	auto monitor = CM::CreateMonitor();

	spdlog::info("memory: {}", monitor->GetMemory().dump());
	spdlog::info("disk {}", monitor->GetDisk().dump());
	spdlog::info("cpu {}", monitor->GetCPU().dump());

	for (;;)
	{
		zmq::message_t request;

		// receive a request from client
		auto anOptional = socket.recv(request, zmq::recv_flags::none);

		spdlog::info("Received is '{}'", request.to_string());

		if (request.to_string() == "RequestBaseInfo")
		{
			// send the reply to the client
			auto data = monitor->GetInfoByJson().dump();
			socket.send(zmq::buffer(data), zmq::send_flags::none);
			spdlog::info("send data:{}", data);
		}
        else if(request.to_string() == "DisksOverview")
        {
            auto data = monitor->DiskInfoOverview().dump();
            socket.send(zmq::buffer(data), zmq::send_flags::none);
            spdlog::info("[DisksOverview]: send data:{}", data);
        }
        else if(request.to_string() == "DisksInfo")
        {
            auto data = monitor->DisksInfo().dump();
            socket.send(zmq::buffer(data), zmq::send_flags::none);
            spdlog::info("[DisksInfo]: send data:{}", data);
        }
        else if(request.to_string() == "DiskIO")
        {
            auto data = monitor->DisksIO().dump();
            socket.send(zmq::buffer(data), zmq::send_flags::none);
            spdlog::info("[DiskIO]: send data:{}", data);
        }
		else
		{
			spdlog::warn("Receive invalid request");
			socket.send(zmq::buffer("invalid request"), zmq::send_flags::none);
		}
	}

	spdlog::error("application crashed!");
	return 0;
}
