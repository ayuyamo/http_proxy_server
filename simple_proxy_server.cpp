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
std::unordered_map<std::string, std::string> cache;
// Ensure that the server does not create more than 20 child processes to avoid overwhelming the server.
const int MAX_CONCURRENT_REQUESTS = 20;
constexpr int TIMEOUT_SECONDS = 300; // 5 minutes timeout

bool server_running = true;
int server_fd;
sem_t max_connections; // Semaphore to limit concurrent connections

std::tuple<std::string, std::string, std::string, std::vector<std::string>> parse_http_request(const std::string& request_data) {
    std::string method, url, http_version;
    std::vector<std::string> headers;

    // Create a string stream to parse the request_data
    std::istringstream ss(request_data);

    // Parse the request line (method, url, and HTTP version)
    ss >> method >> url >> http_version;

    // Parse headers
    std::string line;
    while (std::getline(ss, line) && line != "\r") {
        headers.push_back(line);
    }

    // Print the parsed parameters
    std::cout << "Method: " << method << std::endl;
    std::cout << "URL: " << url << std::endl;
    std::cout << "HTTP Version: " << http_version << std::endl;
    std::cout << "Headers:" << std::endl;
    for (const auto& header : headers) {
        std::cout << header << std::endl;
    }

    return std::make_tuple(method, url, http_version, headers);
}

// Callback function to write response data to a string
size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string handle_client_request(const std::string& url, const std::string& http_version) {
    // Check if the URL is in the cache
    if (cache.find(url) != cache.end()) {
        std::cout << "Cache hit for " << url << "!!!!!" << std::endl;
        return cache[url];
    }
    // Initialize libcurl
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl" << std::endl;
        return "";
    }

    // Set the URL to perform the GET request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Include header information in the response
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

    // Set HTTP version
    if (http_version == "HTTP/1.0") {
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    } else if (http_version == "HTTP/1.1") {
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    } else if (http_version == "HTTP/2.0") {
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
    } else {
        std::cerr << "Unsupported HTTP version: " << http_version << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }

    // Set up callback to write response data
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the GET request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to perform HTTP GET request: " << curl_easy_strerror(res) << std::endl;
    } else { // Cache the response
        cache[url] = response;
    }

    // Clean up
    curl_easy_cleanup(curl);   

    return response;
}

void handle_request(int client_socket, int server_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0); 
    std::string request_data(buffer, bytes_received);
    auto[method, url, http_version, headers] = parse_http_request(request_data);

    // Process the request
    std::string response;
    response = handle_client_request(url, http_version);
    send(client_socket, response.c_str(), response.size(), 0);
    // Close the client socket
    close(client_socket);
    // Release the semaphore after handling the request
    sem_post(&max_connections);
}

void start_proxy_server(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    listen(server_fd, 5);

    sem_init(&max_connections, 0, MAX_CONCURRENT_REQUESTS); // Initialize semaphore

    std::vector<std::thread> threads;
    while(server_running) {
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        int ready = select(server_fd + 1, &read_fds, nullptr, nullptr, &timeout);
        if (ready == -1) {
            std::cerr << "Error in select()" << std::endl;
            break;
        } else if (ready == 0) {
            // Get the current time
            time_t now = time(nullptr);
            // Convert the time to a human-readable format
            std::cout << "Server timed out waiting for client connections at " << ctime(&now) << std::endl;
            break;
        }

        // Print the number of active connections
        int sem_value;
        sem_getvalue(&max_connections, &sem_value);
        std::cout << "Active connections: " << MAX_CONCURRENT_REQUESTS - sem_value << std::endl;

        socklen_t addr_len = sizeof(address);
        int client_fd = accept(server_fd, (struct sockaddr *)&address, &addr_len);
        if (client_fd < 0) {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }

        // Acquire semaphore to limit concurrent requests
        if (sem_wait(&max_connections) != 0) {
            std::cerr << "Error acquiring semaphore" << std::endl;
            close(client_fd);
            continue;
        }
        // Create a new thread to handle the client request
        threads.emplace_back(handle_request, client_fd, server_fd);
    }
    // Join all threads before exiting
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    // Destroy semaphore before exiting
    sem_destroy(&max_connections);
    close(server_fd);
}

void shutdown_server(int signal) {
    std::cout << "Shutting down server..." << std::endl;
    server_running = false;
    close(server_fd); // Close the server socket
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }
    
    int port = std::stoi(argv[1]);

    // Register signal handler for graceful shutdown
    signal(SIGINT, shutdown_server);

    start_proxy_server(port);
    
    return 0;
}

