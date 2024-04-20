# Simple HTTP Proxy Server

This is a simple HTTP proxy server implemented in C++. It forwards HTTP GET requests from clients to the specified server and returns the responses back to the clients.

## Usage

1. **Compile the Code**: Compile the C++ code using a C++ compiler like g++. Make sure you have libcurl installed.

   ```bash
   g++ -o simple_proxy_server simple_proxy_server.cpp -lcurl
   ```
2. **Run the Proxy Server**: Execute the compiled binary and specify the port number on which the proxy server will listen for incoming connections.

    ```
    ./simple_proxy_server <port>
    ```
## Example

- Assuming you compiled the code and started the proxy server on port `8080`, you can configure your web browser or any other HTTP client to use `localhost:8080` as the proxy server. Then, when you make HTTP GET requests through the client, the proxy server will forward those requests to the specified server and return the responses to the client.
Notes

- Ensure that `libcurl` is installed on your system before compiling the code
    - To install libcurl on Debian-based Linux distributions (like Ubuntu), you can use the following command:

    ```
    sudo apt-get install libcurl4-openssl-dev
    ```
    - On Red Hat-based distributions (like Fedora), you can use:

    ```
    sudo yum install libcurl-devel
    ```
    - And on macOS, you can use Homebrew:

    ```
    brew install curl
    ```
- You can modify the code to add more features or improve functionality according to your requirements.