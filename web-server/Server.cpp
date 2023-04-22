#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

int main()
{
    int PORT = 8080; // declarar y definir PORT como un entero con valor 8080
    int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket == -1)
    {
        std::cerr << "socket failed." << std::endl;
        return 1;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = htonl(INADDR_ANY);
    service.sin_port = htons(PORT);

    if (bind(listeningSocket, reinterpret_cast<sockaddr *>(&service), sizeof(service)) == -1)
    {
        std::cerr << "bind failed." << std::endl;
        close(listeningSocket);
        return 1;
    }

    if (listen(listeningSocket, SOMAXCONN) == -1)
    {
        std::cerr << "listen failed." << std::endl;
        close(listeningSocket);
        return 1;
    }

    std::cout << "Server started listening on port " << PORT << "..." << std::endl;

    int clientSocket;
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    char buffer[1024];

    while (true)
    {
        clientSocket = accept(listeningSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
        if (clientSocket == -1)
        {
            std::cerr << "accept failed." << std::endl;
            close(listeningSocket);
            return 1;
        }

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1)
        {
            std::cerr << "recv failed." << std::endl;
            close(clientSocket);
            close(listeningSocket);
            return 1;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Received message: " << buffer << std::endl;

        // Parse the request method and path
        std::string method = "";
        std::string path = "";
        std::string request = buffer;
        char *token = strtok(buffer, " ");
        if (token != NULL)
        {
            method = token;
            token = strtok(NULL, " ");
            if (token != NULL)
            {
                path = token;
            }
        }

        // Handle GET request
if (method.compare("GET") == 0)
{
    // Check if the request path ends with ".jpg"
    if (path.length() >= 4 && path.substr(path.length() - 4) == ".jpg")
    {
        // Open the JPEG file
        std::ifstream file(path.substr(1), std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            // Get the size of the file
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            // Read the file into a buffer
            char *buffer = new char[size];
            if (file.read(buffer, size))
            {
                // Send the JPEG file in the response
                std::string response = "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: image/jpeg\r\n"
                                        "Content-Length: " + std::to_string(size) + "\r\n"
                                        "\r\n";
                send(clientSocket, response.c_str(), response.size(), 0);
                send(clientSocket, buffer, size, 0);
            }
            else
            {
                // Failed to read the file
                std::string response = "HTTP/1.1 500 Internal Server Error\r\n"
                                        "Content-Type: text/html\r\n"
                                        "\r\n"
                                        "<html><body><h1>Internal Server Error</h1></body></html>";
                send(clientSocket, response.c_str(), response.size(), 0);
            }
            delete[] buffer;
        }
        else
        {
            // File not found
            std::string response = "HTTP/1.1 404 Not Found\r\n"
                                    "Content-Type: text/html\r\n"
                                    "\r\n"
                                    "<html><body><h1>404 Not Found</h1></body></html>";
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        file.close();
    }
    else
    {
        // Serve index.html if the request path is "/"
        if (path == "/")
        {
            path = "/index.html";
        }

        // Open the file
        std::ifstream file(path.substr(1));
        if (file.is_open())
        {
            // Read the file into a string
            std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

            // Send the file in the response
            std::string response = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/html\r\n"
                                    "Content-Length: " + std::to_string(content.size()) + "\r\n"
                                    "\r\n" + content;
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        else
        {
            // File not found
            std::string response = "HTTP/1.1 404 Not Found\r\n"
                                    "Content-Type: text/html\r\n"
                                    "\r\n"
                                    "<html><body><h1>404 Not Found</h1></body></html>";
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        file.close();
    }
}


else if (method.compare("POST") == 0) {
    // Process POST request
    int content_length = 0;
    std::string::size_type pos = request.find("Content-Length: ");
    if (pos != std::string::npos) {
        content_length = stoi(request.substr(pos + 16));
    }
    char buffer[content_length];
    recv(listeningSocket, buffer, content_length, 0);
    std::string data(buffer, content_length);
    std::string response = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "\r\n"
                      "<html><body><p>You submitted: " + data + "</p></body></html>";
    send(listeningSocket, response.c_str(), response.size(), 0);
} else {
    // Method not implemented
    std::string response = "HTTP/1.1 501 Not Implemented\r\n"
                      "Content-Type: text/html\r\n"
                      "\r\n"
                      "<html><body><h1>Method not implemented</h1></body></html>";
    send(listeningSocket, response.c_str(), response.size(), 0);
}
}
}