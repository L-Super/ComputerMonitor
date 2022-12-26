//
// Created by Listening on 2022/11/10.
//
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>
#include "raid_monitor_server.h"

int main(int argc, char* argv[])
{
	std::string addr;
	std::string msg;
	if (argc < 2)
	{
		std::cout << "no ip and port,will use default ip and port" << std::endl;
		addr = {"tcp://127.0.0.1:20777"};
		msg = "DisksOverview";
//		return 0;
	}
	else
	{
		std::string ip(argv[1]);
		std::string port(argv[2]);
		addr = {"tcp://" + ip + ":" + port};
		msg = argv[3];
	}

	// initialize the zmq context with a single IO thread
	zmq::context_t context{ 1 };
	// construct a REQ (request) socket and connect to interface
	zmq::socket_t socket{ context, ZMQ_REQ };

	std::cout << "addr is " << addr << std::endl;
//	socket.connect("tcp://localhost:20777");
	socket.connect(addr);
	// set up some static data to send
	const std::string data{ msg };
	for (auto request_num = 0; request_num < 10; ++request_num)
	{
		// initialize a request message
//		zmq::message_t request{ data.cbegin(), data.cend() };
		zmq::message_t request{ data };
		// send the request message
		std::cout << "Sending Hello " << request_num << "..." << std::endl;

		socket.send(request, zmq::send_flags::none);
		// wait for reply from server
		zmq::message_t reply{};
		socket.recv(reply, zmq::recv_flags::none);

		std::cout << "Received " << reply.to_string() << " (" << request_num << ")" << std::endl;
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1s);
	}

//    std::cout<<"receive"<<request_data_from_raid("RequestBaseInfo")<<std::endl;
	return 0;
}
