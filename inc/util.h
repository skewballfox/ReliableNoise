#ifndef UTIL_H
#define UTIL_H
/*
    Author: Joshua Ferguson
    Student_id: 902493976
*/
#include <iostream>
#include "boost/asio.hpp"


inline void parse_commandline_args(int argc, char** argv, std::string& emulator_host_name, int& first_port, int& second_port, std::string& file_name)
{
    std::cout << argv << "\n\n";
    //TODO: add in some error handling
    emulator_host_name = std::string(argv[1]);
    //if (argc >= 2) // not robust but works for the assignment
    first_port = std::strtol(argv[2], nullptr, 10);
    second_port = std::strtol(argv[3], nullptr, 10);
    file_name = std::string(argv[4]);
    std::cout << "provided hostname is " << emulator_host_name << "\n\n";

}
inline boost::asio::ip::address get_emulator_addr(std::string emulator_host_name)
{
    boost::asio::ip::address emulator_addr;
    std::cout << "setting emulator address\n\n";
    std::cout << "provided hostname is " << emulator_host_name << "\n\n";
    if (emulator_host_name == "localhost") {
        emulator_addr = boost::asio::ip::address::from_string(std::string("0.0.0.0"));
    }
    else {
        emulator_addr = boost::asio::ip::address::from_string(std::string(emulator_host_name));
    }
    std::cout << "emulator address is " << emulator_addr << "\n\n";
    return emulator_addr;
}
inline void  set_endpoints(boost::asio::ip::address emulator_addr, int first_port, int second_port, boost::asio::ip::udp::endpoint& first_endpoint, boost::asio::ip::udp::endpoint& second_endpoint)
{

    //TODO: add in some error handling


    first_endpoint = boost::asio::ip::udp::endpoint(emulator_addr, first_port);
    second_endpoint = boost::asio::ip::udp::endpoint(emulator_addr, second_port);
}

//since the setup process for both the client and server are exactly the same
//this method is just a wrapper around the above functions
inline void argv_to_endpoints(int argc, char** argv, boost::asio::ip::udp::endpoint first_endpoint, boost::asio::ip::udp::endpoint second_endpoint)
{

}


inline int int_from_msg(char msg[])
{
    return std::strtol(msg, nullptr, 10);
}

inline char* msg_to_caps(char msg[])
{
    int size = strlen(msg);
    for (int i = 0; i < size;i++) {
        msg[i] = toupper(msg[i]);
    }
    return msg;
}

inline double elapsed_time(time_t start_time)
{
    time_t current_time = time(NULL);
    return difftime(current_time, start_time);
}

inline bool has_timed_out(time_t start_time, double max_time)
{
    return elapsed_time(start_time) > max_time;
}

inline std::unique_ptr<std::ofstream> get_output_file_stream(std::string filename)
{
    auto buffer = std::make_unique<std::ofstream>(filename.c_str(), std::ofstream::trunc);
    return buffer;
}

inline void open_socket(boost::shared_ptr<boost::asio::ip::udp::socket> socket)
{
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    socket->open(boost::asio::ip::udp::v4());
    //allow the socket to be (potentially) shared by both the client and server
    socket->set_option(boost::asio::ip::udp::socket::reuse_address(true));
    //set the timeout value for the socket
    //thanks to https://stackoverflow.com/a/51850018/11019565
    std::cout << "yeet\n\n";
    //socket->set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>(timeout));

    ::setsockopt(socket->native_handle(), SOL_SOCKET, SO_RCVTIMEO, (const void*)&timeout, sizeof timeout);
    ::setsockopt(socket->native_handle(), SOL_SOCKET, SO_SNDTIMEO, (const void*)&timeout, sizeof timeout);

}

#endif