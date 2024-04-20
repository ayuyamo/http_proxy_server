import socket, sys

def send_request(port):
    # Create a socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Connect to the proxy server
    client_socket.connect(('172.20.213.159', port))
    
    # Send the GET request to the proxy server
    request = "GET http://www.google.com/ HTTP/1.0\r\n\r\n"
    client_socket.sendall(request.encode())
    
    # Receive and print the response from the proxy server
    response = client_socket.recv(10000)
    print(response.decode())

    # Close the socket
    client_socket.close()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 client_request.py <port>")
        sys.exit(1)
    
    port = int(sys.argv[1])
    send_request(port)
