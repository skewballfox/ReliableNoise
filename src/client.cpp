/*
    Author: Joshua Ferguson
    Student_id: 902493976
*/
#include <string>
#include <iostream>
//#include <filesystem> //gcc on pluto couldn't find it despite specifying the c++17 standard, which should include it 
#include <fstream>
#include <time.h>
#include "boost/asio.hpp"
#include "util.h"
#include "peer.h"

#include "packet.cpp"

using namespace boost::asio;

std::unique_ptr<std::ifstream> get_file_buffer(std::string filename);
void send_packet(shared_ptr<peer> client, std::unique_ptr<std::ofstream>& send_log, std::unique_ptr<std::ofstream>& recv_log, ip::udp::endpoint en, char msg[4], char ack[4], boost::system::error_code err);
void send_data(std::unique_ptr<std::ifstream>& buffer, std::unique_ptr<std::ofstream>& send_log, std::unique_ptr<std::ofstream>& recv_log, shared_ptr<peer> client, ip::udp::endpoint send_en, boost::system::error_code err);


int main(int argc, char** argv)
{
    //======================================== part 1: preprocessing ================================
    //===============================================================================================

    //handle commandline args
    if (argc != 5) {
        std::cout << "unsupported number of arguments\n\n";
        return 1;
    }
    string emulator_name;
    int send_port, recv_port;
    string input_file;
    parse_commandline_args(argc, argv, emulator_name, send_port, recv_port, input_file);

    //set the values necessary for communication
    auto host_addr = get_emulator_addr(emulator_name);

    ip::udp::endpoint server_endpoint; //the endpoint data will be transmitted to
    ip::udp::endpoint this_endpoint; //the endpoint used by the client to recieve acks

    set_endpoints(host_addr, send_port, recv_port, server_endpoint, this_endpoint);

    auto client = make_shared<peer>(this_endpoint);
    //set up file buffers for handling input and logging
    auto input_file_stream = get_file_buffer(input_file);
    auto sequence_log = get_output_file_stream(std::string("clientseqnum.log"));
    auto ack_log = get_output_file_stream(std::string("clientack.log"));


    //===================================== part 2: doing the work =================================
    //==============================================================================================


    //based off SO answer https://stackoverflow.com/a/5425756 





    boost::system::error_code err;

    send_data(input_file_stream, sequence_log, ack_log, client, server_endpoint, err);

    //===================================== part 3: graceful shutdown =================================
    //=================================================================================================
    //gracefully close file stream and socket
    input_file_stream->close();
    sequence_log->close();
    ack_log->close();




}

std::unique_ptr<std::ifstream> get_file_buffer(std::string filename)
{
    // if (!(std::filesystem::exists(filename))) // confirm file existence
    // {
    //     std::cout << "Sorry, that wasn't a valid file. Please Try again. " << std::endl
    //         << std::endl;
    // }
    auto buffer = std::make_unique<std::ifstream>(filename.c_str());

    if (!buffer->is_open()) {
        std::cout << "Issue opening file" << filename << std::endl;
    }

    return buffer;
}

void send_packet(shared_ptr<peer> client, std::unique_ptr<std::ofstream>& send_log, std::unique_ptr<std::ofstream>& recv_log, ip::udp::endpoint send_en, packet msg, boost::system::error_code err)
{
    //timer for calculating how much time has elapsed
    auto timeout = boost::posix_time::seconds(2);
    //bool for loop break
    //bool is_acked = false;


    char send_buf[37];

    //size of ack is 6 chars if no data

    char log_buf[2 + sizeof(char)];

    packet ack = packet(0, 0, 0, NULL);
    memset(send_buf, 0, 6 + msg.getLength());
    msg.serialize(send_buf);



    std::cout << "sending packet: " << send_buf << "\n\n";
    do {
        client->send_to(boost::asio::buffer(send_buf, sizeof(send_buf) + 1), send_en, timeout, err);
        if (err) {
            std::cout << "oops: " << err << "\n\n";
            continue;
        }

        //throw boost::system::system_error(err);
        std::cout << "packet sent\n\n";
        //TODO: verify if pluto has c++20, may need to switch to boost if only upto 17
        //see https://stackoverflow.com/a/10410159/11019565
        std::sprintf(log_buf, "%d\n", msg.getSeqNum());
        send_log->write(log_buf, 2);
        std::cout << "awaiting ack\n\n";
        char recv_buf[6];
        client->receive(boost::asio::buffer(recv_buf, 6), timeout, err);
        if (err) {
            std::cout << "error while waiting on ack: " << err << "\n reattempting send\n\n";
            continue;
        }
        std::cout << "response received: " << recv_buf << "\n\n";


        ack.deserialize(recv_buf);
        //throw boost::system::system_error(err);
        std::sprintf(log_buf, "%d\n", ack.getSeqNum());
        recv_log->write(log_buf, 2);
        //see https://stackoverflow.com/questions/48124690/why-is-my-array-printing-out-a-duplicate
        if (ack.getType() == 0 && msg.getSeqNum() == ack.getSeqNum()) {
            std::cout << "packet acked\n\n";
            break;
        }
        else if (ack.getType() == 2) {
            std::cout << "server sent EOT, but client was waiting on ack\n";
            std::cout << "exiting anyway\n\n";
            exit(1);
        }

    } while (true);


}

void send_data(std::unique_ptr<std::ifstream>& buffer, std::unique_ptr<std::ofstream>& send_log, std::unique_ptr<std::ofstream>& recv_log, shared_ptr<peer> client, ip::udp::endpoint send_en, boost::system::error_code err)
{
    char buf[30];


    packet* msg = NULL;
    int seqnum = 0;
    int buf_len = 0;

    //process file
    std::cout << "starting send\n\n";
    do {
        buf_len = buffer->readsome(buf, 30);
        msg = new packet(1, seqnum, buf_len, buf);
        send_packet(client, send_log, recv_log, send_en, *msg, err);
        seqnum = (seqnum == 0) ? 1 : 0;
    } while (buffer->peek() != EOF);

    //send EOT packet
    char EOT_buf[6];
    memset(EOT_buf, 0, 6);
    msg = new packet(2, seqnum, 0, NULL);
    msg->serialize(EOT_buf);
    client->send_to(boost::asio::buffer(EOT_buf, 37), send_en, boost::posix_time::seconds(10), err);
    if (err)
        throw boost::system::system_error(err);
    std::cout << "am i reached?\n\n";
}

