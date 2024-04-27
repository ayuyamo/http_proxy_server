#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h> 
#include <netinet/in.h>
#include <string>
#include <vector>
#include <sstream>
#include <tuple>
#include <curl/curl.h>
#include <ctime>
#include <unordered_map>
#include <sys/wait.h>
#include <thread> // Include for multithreading
#include <semaphore.h> // Include semaphore header
#include <ctime>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>

const int BUFFER_SIZE = 1024;

// Define a cache to store cached responses
extern std::unordered_map<std::string, std::string> cache;
// Ensure that the server does not create more than 20 child processes to avoid overwhelming the server.
extern const int MAX_CONCURRENT_REQUESTS;
constexpr int TIMEOUT_SECONDS = 300; // 5 minutes timeout

extern bool server_running;
extern int server_fd;
extern sem_t max_connections; // Semaphore to limit concurrent connections

std::tuple<std::string, std::string, std::string, std::vector<std::string>> parse_http_request(const std::string& request_data);
size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data);
std::string handle_client_request(const std::string& url, const std::string& http_version);
void handle_request(int client_socket, int server_socket);
void start_proxy_server(int port);
void shutdown_server(int signal);

#endif // PROXY_SERVER_H
