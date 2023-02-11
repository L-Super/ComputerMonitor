//
// Created by Listening on 2022/11/10.
//
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>

/**
 * @brief 非阻塞
 * 如果服务端不在线，socket.recv()将无限等待下去，因此可改为非阻塞，设置超时，避免死等情况。
 */
void no_block_socket(const std::string& addr, const std::string& msg)
{
	// initialize the zmq context with a single IO thread
	zmq::context_t context{1};
	// construct a REQ (request) socket and connect to interface
	zmq::socket_t socket{context, zmq::socket_type::req};
	// 在关闭套接字时等待发送的消息的时间，以避免 recv() 函数阻塞。这将导致在关闭套接字时立即丢弃任何未发送的消息，而不等待它们被传输。
	// 当 ZMQ_LINGER 选项设置为一个正整数（例如1000）时，代表在关闭连接时，等待这个数值指定的毫秒数，以确保所有未发送的数据都能被发送。
	socket.setsockopt(ZMQ_LINGER, 0);

	socket.connect(addr);

	zmq::pollitem_t items[] = {
		{socket, 0, ZMQ_POLLIN, 0}};

	// set up some static data to send
	const std::string data{msg};

	for (auto request_num = 0; request_num < 10; ++request_num)
	{
		// initialize a request message
		//		zmq::message_t request{ data.cbegin(), data.cend() };
		zmq::message_t request{data};
		// send the request message
		std::cout << "Sending Hello " << request_num << "..." << std::endl;

		auto op = socket.send(request, zmq::send_flags::none);
		// C++11 optional
		if (op.has_value())
		{
			std::cout << "option value: " << op.value() << std::endl;
		}

		// 1s timeout in milliseconds
		zmq::poll(&items[0], 1, 1000);
		zmq::message_t reply;
		if (items[0].revents & ZMQ_POLLIN)
		{
			// wait for reply from server
			auto result = socket.recv(reply);
			std::cout << "Received Reply " << request_num << " [" << reply.to_string() << "]" << std::endl;
		}
		else
		{
			std::cout << "Receive timeout!" << std::endl;
			socket.close();
			std::cout<<"socket close\n";
			break;
		}
	}
}

/**
 * @brief 阻塞
 */
void block_socket(const std::string& addr, const std::string& msg)
{
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
}

int main(int argc, char* argv[])
{
	std::string addr;
	std::string msg;
	if (argc < 2)
	{
		std::cout << "no ip and port,will use default ip and port" << std::endl;
		addr = {"tcp://127.0.0.1:20777"};
		msg = "DisksOverview";
	}
	else
	{
		std::string ip(argv[1]);
		std::string port(argv[2]);
		addr = {"tcp://" + ip + ":" + port};
		msg = argv[3];
	}

	no_block_socket(addr, msg);

	return 0;
}
