import socket
import unittest
import threading

MAX_THREADS = 20

class TestProxyServer(unittest.TestCase):
    def setUp(self):
        self.port = 9998  # Change to port where the proxy server is running
        self.num_clients = 50  # Number of concurrent clients for testing
        
    def send_request(self):
        # Create a socket
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Connect to the proxy server
        server_address = ('127.0.0.1', self.port)  # Assuming proxy server runs on localhost
        client_socket.connect(server_address)

        # Construct the GET request
        request = "GET http://www.google.com/ HTTP/1.0\r\n\r\n"

        # Send the GET request to the proxy server
        client_socket.sendall(request.encode())

        # Receive and return the response from the proxy server
        response = client_socket.recv(4096).decode()

        # Close the socket
        client_socket.close()

        return response

    def test_proxy_server_response(self):
        response = self.send_request()
        self.assertIn("HTTP/1.0 200 OK", response)
        self.assertIn("<title>Google</title>", response)
    
    def test_multiple_clients(self):
        # Test with multiple concurrent clients
        threads = []
        for _ in range(self.num_clients):
            thread = threading.Thread(target=self.send_request)
            threads.append(thread)
            thread.start()
            
        # Count the number of active threads
        active_threads = threading.active_count()
        # Assert that the number of active threads is 20 or lower
        self.assertLessEqual(active_threads, self.num_clients)

        for thread in threads:
            thread.join()
        
        for thread in threads:
            self.assertFalse(thread.is_alive())    
        
    def test_cache_logic(self):
        # Test cache logic by sending the same request multiple times
        # Verify that the response is retrieved from the cache after the first request
        response1 = self.send_request()
        response2 = self.send_request()  # Send the same request again
        self.assertEqual(response1, response2)

if __name__ == "__main__":
    unittest.main()
