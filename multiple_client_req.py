import socket
import threading
import sys

def send_request(server_ip, server_port):
    # Create a socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # Connect to the server
        client_socket.connect((server_ip, server_port))
        
        # Send a dummy HTTP request
        request = "GET http://www.google.com/ HTTP/1.0\r\n\r\n"
        client_socket.sendall(request.encode())
        
        # Receive and print the response
        response = client_socket.recv(100000).decode()
        return response
    except Exception as e:
        print("Error:", e)
    finally:
        # Close the socket
        client_socket.close()

def main():
    if len(sys.argv) != 3:
        print("Usage: python client.py <port> <num_clients>")
        return
    
    server_port = int(sys.argv[1])
    server_ip = "127.0.0.1"

    # Send requests concurrently
    num_requests = int(sys.argv[2]) 
    threads = []
    for i in range(num_requests):
        print("Sending request", i + 1)
        thread = threading.Thread(target=send_request, args=(server_ip, server_port))
        threads.append(thread)
        thread.start()
    
    # Wait for all threads to complete
    for thread in threads:
        thread.join()
        print("Request completed")

if __name__ == "__main__":
    main()
