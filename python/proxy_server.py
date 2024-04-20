import socket, requests, sys

def start_proxy_server(port):
    # Create a socket
    proxy_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Bind the socket to the port
    proxy_socket.bind(('0.0.0.0', port))
    
    # Listen for incoming connections
    proxy_socket.listen(5)
    
    print(f"Proxy server listening on port {port}...")
    
    while True:
        # Accept incoming connection
        client_socket, _ = proxy_socket.accept()
        
        # Spawn a new process to handle the request
        handle_request(client_socket)

def handle_request(client_socket):
    # Read data from the client socket
    request_data = client_socket.recv(1024).decode('utf-8')
    
    # Parse the request
    method, url, http_version, headers = parse_http_request(request_data)
    
    # Check if the request is properly formatted
    if method is None or url is None or http_version is None:
        # Respond with Bad Request (400)
        client_socket.sendall(b"HTTP/1.0 400 Bad Request\r\n\r\n")
        client_socket.close()
        return
    
    # Process the request
    if method == 'GET':
        # Handle GET request
        handle_get_request(url, headers, client_socket)
    else:
        # Respond with Not Implemented (501) for other methods
        client_socket.sendall(b"HTTP/1.0 501 Not Implemented\r\n\r\n")
        client_socket.close()

def parse_http_request(request_data):
    # Split request data into lines
    lines = request_data.split('\r\n')
    
    # Parse request line
    request_line = lines[0].split()
    if len(request_line) != 3:
        return None, None, None, None  # Invalid request format
    method, url, http_version = request_line
    
    # Parse headers
    headers = {}
    for line in lines[1:]:
        if line.strip() == '':
            break  # End of headers
        header_name, header_value = line.split(': ', 1)
        headers[header_name] = header_value
    
    return method, url, http_version, headers


def handle_get_request(url, headers, client_socket):
    # Send HTTP GET request to the target server
    try:
        response = requests.get(url, headers=headers)
        status_line = f"HTTP/1.0 {response.status_code} {response.reason}\r\n"
        header_lines = "\r\n".join([f"{header}: {value}" for header, value in response.headers.items()])
        response_data = status_line.encode('utf-8') + header_lines.encode('utf-8') + b"\r\n\r\n" + response.content
    except requests.RequestException as e:
        # Handle request errors
        response_data = b"HTTP/1.0 500 Internal Server Error\r\n\r\n"
    
    # Send the response back to the client
    client_socket.sendall(response_data)
    client_socket.close()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 proxy_server.py <port>")
        sys.exit(1)
    
    port = int(sys.argv[1])
    start_proxy_server(port)
