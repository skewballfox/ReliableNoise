/*
    Author: Joshua Ferguson
    Student_id: 902493976
*/
#include <string>
#include <iostream>
#include <time.h>
#include <tuple>
#include <fstream>

#include "boost/asio.hpp"
#include "util.h"
#include "packet.cpp"
#include "peer.h"

using namespace boost::asio;


void recv_data(std::unique_ptr<std::ofstream>& buffer, std::unique_ptr<std::ofstream>& msg_log, std::shared_ptr<peer> server, ip::udp::endpoint en, boost::system::error_code err);


int main(int argc, char** argv)
{
    //======================================== part 1: preprocessing ================================
    //===============================================================================================

    //handle commandline args
    if (argc != 5) {
        std::cout << "unsupported number of arguments: " << argc << "\n\n";
        return 1;
    }
    std::string emulator_name;
    int recv_port, send_port;
    std::string output_file;

    parse_commandline_args(argc, argv, emulator_name, recv_port, send_port, output_file);

    //set the values necessary for communication
    boost::asio::ip::address host_addr = get_emulator_addr(emulator_name);

    ip::udp::endpoint this_endpoint; //the endpoint data will be transmitted to
    ip::udp::endpoint client_endpoint; //the endpoint used by the client to recieve acks

    set_endpoints(host_addr, recv_port, send_port, this_endpoint, client_endpoint);

    auto server = make_shared<peer>(this_endpoint);


    boost::system::error_code err;




    auto out_buffer = get_output_file_stream(output_file);
    auto msg_log = get_output_file_stream(std::string("arrival.log"));

    recv_data(out_buffer, msg_log, server, client_endpoint, err);

    out_buffer->close();
    msg_log->close();



    return 0;

}



void recv_data(std::unique_ptr<std::ofstream>& buffer, std::unique_ptr<std::ofstream>& msg_log, std::shared_ptr<peer> server, ip::udp::endpoint en, boost::system::error_code err)
{
    //TODO: finish this function
    auto timeout = boost::posix_time::seconds(2);
    int timeout_count = 0;
    char* recv_buf = new char[37]();

    char* tmp = new char[30];
    char log_buf[2 + sizeof(char)];
    //to avoid writing duplicate data
    int seqnum = 0;

    memset(tmp, '0', 30 * sizeof(char));
    packet msg_packet = packet(1, 0, 0, tmp);
    packet* ack_packet = NULL;
    std::cout << "starting receive function\n\n";
    int msg_len;
    std::cout << "client endpoint: " << en << "\n\n";
    do {
        //receive data from client
        server->receive(boost::asio::buffer(recv_buf, 37), timeout, err);
        if (err) {
            std::cout << "error encountered: " << err << "\n\n";
            if (err.value() == 125) {
                std::cout << "encountered timeout, checking if counter at exit threshold\n\n";
                //to avoid never writing output if it hasn't synced yet
                if (timeout_count == 5) {
                    std::cout << "syncing file output\n\n";
                    buffer->flush();
                    msg_log->flush();

                    //necessary if you want the server to discontinue after 
                    //EOT, which never makes its way to the server due to an 
                    //error in emulator which routes packets with type 2 from client to client
                    break;
                }
                else {
                    timeout_count += 1;
                }
            }
            continue;
        }
        else {
            timeout_count = 0;
        }
        std::cout << "received packet: " << recv_buf << "\n\n";

        msg_packet.deserialize(recv_buf);
        std::cout << "packet length" << msg_packet.getLength() << "\n\n";
        //log the data






        //send ack to client
        if (msg_packet.getType() == 1) {
            //moved to declaring in loop to avoid residue from previous iterations
            char send_buf[6];
            memset(send_buf, 0, 6);
            //if we should log all arrivals rather than just the unacked ones
            //then writing to the log should go here
            //msg_log->write(log_buf, 2);
            if (msg_packet.getSeqNum() == seqnum) {
                std::cout << "new msg, writing to files\n\n";
                std::sprintf(log_buf, "%d\n", msg_packet.getSeqNum());
                buffer->write(msg_packet.getData(), msg_packet.getLength());
                msg_log->write(log_buf, 2);
                ack_packet = new packet(0, seqnum, 0, NULL);
                ack_packet->serialize(send_buf);
                seqnum = (seqnum == 0) ? 1 : 0;
            }
            else { //resend the last sequence number
                int tmp = (seqnum == 0) ? 1 : 0;
                ack_packet = new packet(0, tmp, 0, NULL);
                ack_packet->serialize(send_buf);
            }

            std::cout << "send buf: " << send_buf << "\n\n";
            server->send_to(boost::asio::buffer(send_buf, 6), en, timeout, err);
            if (err) {
                //125 in case of timeout
                std::cout << "error code: " << err << "\n\n";
            }

            std::cout << "sending ack\n\n";




        }
        else {
            std::cout << msg_packet.getType() << "\n exiting now\n\n";
            break;
        };
    } while (true);//value should always be one, even after failed transmission until client sends EOT
    std::cout << "that's all, folks\n\n";
    //send EOT server-side, just in case
    packet EOT(2, seqnum,0, NULL);
    char send_buf[6];
    memset(send_buf, 0, 6);
    EOT.serialize(send_buf);
    server->send_to(boost::asio::buffer(send_buf, 6), en, timeout, err);
    //just in case
    delete ack_packet;
    //std::cout << "client endpoint: " << client << "\n\n";
}
