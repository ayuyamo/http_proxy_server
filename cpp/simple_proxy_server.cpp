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

const int BUFFER_SIZE = 1024;
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

  return std::make_tuple(method, url, http_version, headers);
}

// Callback function to write response data to a string
size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string handle_get_request(const std::string& url, const std::string& http_version) {
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

    // Print the parsed parameters
    std::cout << "Method: " << method << std::endl;
    std::cout << "URL: " << url << std::endl;
    std::cout << "HTTP Version: " << http_version << std::endl;
    std::cout << "Headers:" << std::endl;
    for (const auto& header : headers) {
        std::cout << header << std::endl;
    }

    // Process the request
    std::string response;
    response = handle_get_request(url, http_version);
    send(client_socket, response.c_str(), response.size(), 0);
}

void start_proxy_server(int port) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  bind(server_fd, (struct sockaddr *)&address, sizeof(address));

  listen(server_fd, 5);

  while(true) {
    socklen_t addr_len = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr *)&address, &addr_len);
    handle_request(client_fd, server_fd);
  }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }
    
    int port = std::stoi(argv[1]);
    start_proxy_server(port);
    
    return 0;
}