//
// Created by LMR on 2022/11/23.
//

#ifndef RAIDMONITORSERVER_RAID_MONITOR_SERVER_H
#define RAIDMONITORSERVER_RAID_MONITOR_SERVER_H

#include <string>
#include <zmq.hpp>

template<typename String>
std::string request_data_from_raid(String ip, String port, String message) {
    zmq::context_t context{1};
    zmq::socket_t socket{ context, ZMQ_REQ };
    std::string addr = {"tcp://"};
    addr.append(ip);
    addr.append(":");
    addr.append(port);
    socket.connect(addr);
    zmq::message_t request{message};

    socket.send(request, zmq::send_flags::none);
    // wait for reply from server
    zmq::message_t reply{};
    socket.recv(reply, zmq::recv_flags::none);

    return reply.to_string();
}
template <typename String>
std::string request_data_from_raid(String message) {
    zmq::context_t context{1};
    zmq::socket_t socket{ context, ZMQ_REQ };
    //TODO:read ip and port from configuration file
//    std::string addr = {"tcp://"};
//    addr.append(ip);
//    addr.append(":");
//    addr.append(port);
    std::string addr{"tcp://127.0.0.1:20777"};
    socket.connect(addr);

    std::string tmp{message};
    zmq::message_t request(tmp);

    socket.send(request, zmq::send_flags::none);
    // wait for reply from server
    zmq::message_t reply{};
    socket.recv(reply, zmq::recv_flags::none);

    return reply.to_string();
}
//std::string request_data_from_raid(const std::string& message) {
//    zmq::context_t context{1};
//    zmq::socket_t socket{ context, ZMQ_REQ };
//    //TODO:read ip and port from configuration file
////    std::string addr = {"tcp://"};
////    addr.append(ip);
////    addr.append(":");
////    addr.append(port);
//    std::string addr{"tcp://127.0.0.1:20777"};
//    socket.connect(addr);
//    zmq::message_t request(message);
//
//    socket.send(request, zmq::send_flags::none);
//    // wait for reply from server
//    zmq::message_t reply{};
//    socket.recv(reply, zmq::recv_flags::none);
//
//    return reply.to_string();
//}

std::string request_disk_overview_from_raid()
{
    return request_data_from_raid("DisksOverview");
}
std::string request_disks_info_from_raid()
{
    return request_data_from_raid("DisksInfo");
}
std::string request_disk_io_from_raid()
{
    return request_data_from_raid("DiskIO");
}

#endif //RAIDMONITORSERVER_RAID_MONITOR_SERVER_H
