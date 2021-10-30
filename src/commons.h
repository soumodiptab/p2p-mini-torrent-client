#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;
/**
 * @brief Header file to provide library for:
 * 1. socket connectivity using wrappers
 * 2. logging in console/logfile
 */
string command_upload_file = "upload_file";
string command_download_file = "download_file";
string command_create_user = "create_user";
string command_login = "login";
string command_create_group = "create_group";
string command_join_group = "join_group";
string command_print = "print";
string command_change_color = "change_color";
/**
 * @brief Print logs |
 *  0 - no output printing
 *  1 - only console
 *  2 - only logs
 *  3 - both
 */
int logging_level = 1;
string log_file;
/**
 * @brief backlog connections:
 * maximum length to which the queue of pending connections for sockfd may grow
 */
const int constants_connection_backlog = 10;
const string constants_socket_failure = "Failed to open socket";
const string constants_socket_binding_failure = "Failed to bind socket";
const string constants_socket_conn_failure = "Failed to connect socket";
const string constants_socket_listen_failure = "Failed to listen to socket";
const string constants_socket_recv_failure = "Failed to recieve message";
const string constants_socket_send_failure = "Failed to send message";
const string constants_socket_empty_reply = "Incorrect reply";
const string constants_client_disconnected = "Opposite End has disconnected";
const string cosntants_client_connected = "Client has connected";

//-----------------------------------------------------------------------------------
const int constants_message_buffer_limit = 1024;
/**
 * @brief ip | port
 * 
 */
pair<string, string> tracker_1;
pair<string, string> tracker_2;

vector<string> colors = {
    "\033[34m",
    "\033[31m",
    "\033[36m",
    "\033[32m",
    "\033[33m",
    "\033[35m",
};

/**
 * @brief Extracts the ip and port from the input string
 * 
 * @param ip_port 
 * @return pair<string, string> 
 */
pair<string, string> read_socket_input(string ip_port)
{
    pair<string, string> socket_pair;
    socket_pair.first = ip_port.substr(0, ip_port.find(":"));
    socket_pair.second = ip_port.substr(ip_port.find(":") + 1);
    return socket_pair;
}
/**
 * @brief Reads the tracker file and extracts the tracker ip_port
 * 
 * @param path 
 */
void read_tracker_file(string path)
{
    string temp;
    ifstream file(path);
    getline(file, temp);
    tracker_1.first = temp.substr(0, temp.find(":"));
    tracker_1.second = temp.substr(temp.find(":") + 1);
}
string path_processor(string path)
{
    string x;
    return x;
}
/**
 * @brief Writes to  a log file with timestamp and thread-id
 * 
 * @param message 
 */
void write_to_log(const std::string &message)
{
    ofstream file_out;
    file_out.open(log_file, std::ios_base::app);
    pthread_t self = pthread_self();
    time_t now = time(0);
    string date_time = ctime(&now);
    date_time.pop_back();
    string final_message = "[" + date_time + "] " + "[" + to_string(self) + "][ " + message + " ]";
    file_out << final_message << endl;
}
/**
 * @brief Selects between displaying log on console/log file
 * 
 * @param message 
 */
void log(string message)
{
    switch (logging_level)
    {
    case 0:
        break;
    case 2:
        write_to_log(message);
        break;
    case 3:
        write_to_log(message);
    default:
        cout << message << endl;
    }
}
void set_log_file(string path)
{
    log_file = path;
}
/**
 * @brief  calling this api turns the tracker/peer into a server
 * @param pair<ip_address,port> : own ip and port
 * @return socket file descriptor
 */
int server_setup(pair<string, string> socket_pair)
{
    struct addrinfo query;
    struct addrinfo *socket_addr;
    memset(&query, 0, sizeof(query));
    query.ai_socktype = SOCK_STREAM; // Tcp connection protocol
    query.ai_family = AF_INET;       // Ipv4 address family
    //it queries the sockets and gets a list of socket address structures with protocol,family type specified and stores it in the socket address pointer
    if (getaddrinfo(socket_pair.first.c_str(), socket_pair.second.c_str(), &query, &socket_addr) != 0)
    {
        log(constants_socket_failure + " : " + socket_pair.first + " " + socket_pair.second);
        return -1;
    }
    int socket_file;
    if ((socket_file = socket(socket_addr->ai_family, socket_addr->ai_socktype, socket_addr->ai_protocol)) < 1)
    {
        log(constants_socket_failure + " : " + socket_pair.first + " " + socket_pair.second);
        return -1;
    }
    //Clear the pointer
    //returns -1 on error
    if (bind(socket_file, socket_addr->ai_addr, socket_addr->ai_addrlen) == -1)
    {
        log(constants_socket_binding_failure + " : " + socket_pair.first + " " + socket_pair.second);
        return -1;
    }
    //Marks the socket as a passive socket<<recieve incoming connections
    if (listen(socket_file, constants_connection_backlog) == -1)
    {
        log(constants_socket_listen_failure + " : " + socket_pair.first + " " + socket_pair.second);
        return -1;
    }
    freeaddrinfo(socket_addr);
    return socket_file;
}

/**
 * @brief calling this api turns the tracker/peer into a client
 * @param pair<ip_address,port> : ip  and port of reciever(server)
 * @return socket file descriptor
 */
int client_setup(pair<string, string> socket_pair)
{
    struct addrinfo query;
    struct addrinfo *socket_addr;
    memset(&query, 0, sizeof(query));
    query.ai_socktype = SOCK_STREAM; // Tcp connection protocol
    query.ai_family = AF_INET;       // Ipv4 address family
    if (getaddrinfo(socket_pair.first.c_str(), socket_pair.second.c_str(), &query, &socket_addr) != 0)
    {
        log(constants_socket_failure + " : " + socket_pair.first + " " + socket_pair.second);
        return -1;
    }
    int socket_file;
    socket_file = socket(socket_addr->ai_family, socket_addr->ai_socktype, socket_addr->ai_protocol);

    //Opens a connection to the socket
    if (connect(socket_file, socket_addr->ai_addr, socket_addr->ai_addrlen) == -1)
    {
        close(socket_file);
        log(constants_socket_conn_failure + " : " + socket_pair.first + " " + socket_pair.second);
        return -1;
    }

    freeaddrinfo(socket_addr);
    return socket_file;
}
/**
 * @brief Sends a message to the opposite end 
 * 
 * @param socket_fd
 * @param message 
 * @return status
 * @throws exception incase unable to send due to socket closure
 */
int socket_send(int socket_fd, string message)
{
    if (send(socket_fd, message.c_str(), message.size(), 0) == -1)
    {
        log(constants_socket_send_failure);
        throw(constants_socket_send_failure);
    }
    return 0;
}
/**
 * @brief Waits to recieve message from the socket
 * 
 * @param socket_fd 
 * @return string 
 * @throws exception incase client forcefully disconnects
 */
string socket_recieve(int socket_fd)
{
    char buff[constants_message_buffer_limit];
    int bytes_recieved;
    if ((bytes_recieved = recv(socket_fd, buff, constants_message_buffer_limit, 0)) == -1)
    {
        log(constants_socket_recv_failure);
        throw(constants_socket_recv_failure);
    }
    else if (bytes_recieved == 0)
    {
        throw(constants_client_disconnected);
    }
    string message(buff);
    string extract = message.substr(0, bytes_recieved);
    return extract;
}
/**
 * @brief Parses command input and convert into tokens
 * 
 * @param input 
 * @return vector<string> 
 */
vector<string> input_parser(string &input)
{
    vector<string> tokens;
    string temp_str = "";
    for (char c : input)
    {
        if (c == ' ')
        {
            tokens.push_back(temp_str);
            temp_str = "";
        }
        else
        {
            temp_str += c;
        }
    }
    if (!temp_str.empty())
        tokens.push_back(temp_str);
    return tokens;
}
/**
 * @brief Packs message into a suitable format to be sent over socket
 * 
 * @param tokens 
 * @return string 
 */
string pack_message(vector<string> &tokens)
{
    string message = "";
    for (auto t : tokens)
    {
        message.append("|");
        message.append(t);
    }
    return message;
}
/**
 * @brief Unpacks message at reciever end
 * 
 * @param packed_message 
 * @return vector<string> 
 */
vector<string> unpack_message(string &packed_message)
{
    vector<string> tokens;
    string temp_str = "";
    for (char c : packed_message)
    {
        if (c == '|')
        {
            tokens.push_back(temp_str);
            temp_str = "";
        }
        else
        {
            temp_str += c;
        }
    }
    if (!temp_str.empty())
        tokens.push_back(temp_str);
    return tokens;
}