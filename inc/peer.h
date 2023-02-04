#ifndef PEER_H
#define PEER_H
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <iostream>

using boost::asio::deadline_timer;
using boost::asio::ip::udp;

/*
    Author: Joshua Ferguson
    Student_id: 902493976
*/
//originally copied from:
//https://www.boost.org/doc/libs/1_52_0/doc/html/boost_asio/example/timeouts/blocking_udp_client.cpp
//renamed to peer because I'm using this for both the client and server and only using this to wrap the async send and receive options
class peer
{
public:
    peer(const udp::endpoint& recv_en)
        : socket_(io_service_, recv_en),
        deadline_(io_service_)
    {

        // No deadline is required until the first socket operation is started. We
        // set the deadline to positive infinity so that the actor takes no action
        // until a specific deadline is set.
        deadline_.expires_at(boost::posix_time::pos_infin);

        // Start the persistent actor that checks for deadline expiry.
        check_deadline();
    }

    std::size_t receive(const boost::asio::mutable_buffer& buffer,
        boost::posix_time::time_duration timeout, boost::system::error_code& ec)
    {
        // Set a deadline for the asynchronous operation.
        deadline_.expires_from_now(timeout);

        // Set up the variables that receive the result of the asynchronous
        // operation. The error code is set to would_block to signal that the
        // operation is incomplete. Asio guarantees that its asynchronous
        // operations will never fail with would_block, so any other value in
        // ec indicates completion.
        ec = boost::asio::error::would_block;
        std::size_t length = 0;

        // Start the asynchronous operation itself. The handle_receive function
        // used as a callback will update the ec and length variables.
        socket_.async_receive(boost::asio::buffer(buffer),
            boost::bind(&peer::handle_receive, _1, _2, &ec, &length));

        // Block until the asynchronous operation has completed.
        do io_service_.run_one(); while (ec == boost::asio::error::would_block);

        return length;
    }

    std::size_t send_to(const boost::asio::mutable_buffer& buffer, udp::endpoint& send_en,
        boost::posix_time::time_duration timeout, boost::system::error_code& ec)
    {
        // Set a deadline for the asynchronous operation.
        deadline_.expires_from_now(timeout);

        // Set up the variables that receive the result of the asynchronous
        // operation. The error code is set to would_block to signal that the
        // operation is incomplete. Asio guarantees that its asynchronous
        // operations will never fail with would_block, so any other value in
        // ec indicates completion.
        ec = boost::asio::error::would_block;
        std::size_t length = 0;

        // Start the asynchronous operation itself. The handle_receive function
        // used as a callback will update the ec and length variables.
        socket_.async_send_to(boost::asio::buffer(buffer), send_en,
            boost::bind(&peer::handle_send, _1, _2, &ec, &length));

        // Block until the asynchronous operation has completed.
        do io_service_.run_one(); while (ec == boost::asio::error::would_block);

        return length;
    }

private:
    void check_deadline()
    {

        if (deadline_.expires_at() <= deadline_timer::traits_type::now()) {

            socket_.cancel();

            // There is no longer an active deadline. The expiry is set to positive
            // infinity so that the actor takes no action until a new deadline is set.
            deadline_.expires_at(boost::posix_time::pos_infin);
        }

        // Put the actor back to sleep.
        deadline_.async_wait(boost::bind(&peer::check_deadline, this));
    }

    static void handle_receive(
        const boost::system::error_code& ec, std::size_t length,
        boost::system::error_code* out_ec, std::size_t* out_length)
    {
        *out_ec = ec;
        *out_length = length;
    }
    static void handle_send(const boost::system::error_code& ec, std::size_t length,
        boost::system::error_code* out_ec, std::size_t* out_length)
    {
        *out_ec = ec;
        *out_length = length;
    }
private:
    boost::asio::io_service io_service_;
    udp::socket socket_;
    deadline_timer deadline_;

};
#endif