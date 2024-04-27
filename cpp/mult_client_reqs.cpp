#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>

const int BUFFER_SIZE = 100000;

void send_request(int port) {
    // Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    std::cerr << "Client socket created successfully\n";
    if (client_socket == -1) {
        std::cerr << "Error: Could not create socket\n";
        return;
    }
    
    // Connect to the proxy server
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Assuming proxy server runs on localhost
    server_address.sin_port = htons(port);
    if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error: Could not connect to server\n";
        close(client_socket);
        return;
    }
    
    // Send the GET request to the proxy server
    std::string request = "GET http://www.google.com/ HTTP/1.0\r\n\r\n";
    if (send(client_socket, request.c_str(), request.length(), 0) == -1) {
        std::cerr << "Error: Could not send request\n";
        close(client_socket);
        return;
    }

    // Receive and print the response from the proxy server
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        std::cerr << "Error: Could not receive response\n";
        close(client_socket);
        return;
    }
    std::cout << std::string(buffer, bytes_received) << std::endl;
    
    // Close the socket
    close(client_socket);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <num_clients>\n";
        return 1;
    }
    
    int port = std::stoi(argv[1]);
    int num_clients = std::stoi(argv[2]);
    
    for (int i = 0; i < num_clients; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            send_request(port);
            exit(0);
        } else if (pid < 0) {
            // Error forking
            std::cerr << "Error: Could not fork process\n";
        }
    }
    
    // Wait for all child processes to complete
    for (int i = 0; i < num_clients; ++i) {
        wait(NULL);
    }
    
    return 0;
}
