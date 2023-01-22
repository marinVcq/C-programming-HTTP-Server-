#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define PORT 80
#define WEBROOT "./webroot"
#define BUFFER_SIZE 1024
#define BACKLOG 10

void handle_connection(SOCKET socket, SOCKADDR_IN *clientAddr);
void report(SOCKADDR_IN *serverAdress);

int send_header(SOCKET socket, char *buffer)
{
    int numbytes;
    int buffer_size = strlen(buffer);

    numbytes = send(socket, buffer, buffer_size, 0);
    if (numbytes == SOCKET_ERROR)
        return 0;

    printf("Send Header succesfully: %s\n", buffer);
    return 1;
}

void reset_connection(SOCKET socket, SOCKADDR_IN serverAddr)
{
    // blablalba
}

int main(void)
{
    WSADATA wsaData = {0};
    SOCKET serverSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    SOCKADDR_IN serverAdress, clientAdress = {0};
    WORD wVersionRequested = MAKEWORD(2, 2);
    BOOL listening = FALSE;
    socklen_t sin_size = sizeof(SOCKADDR_IN);

    int gAdresseFamily = AF_UNSPEC,
        gSocketType = SOCK_STREAM,
        gProtocol = IPPROTO_TCP;

    int err;
    BOOL iOptVal = TRUE;
    int iOptLen = sizeof(BOOL);

    // Socket setup: Initialize winsock2
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        printf("WSAStartup() failed with error code %d\n", err);
        WSACleanup();
        return -1;
    }

    // Socket Setup: Create the socket, specify protocol family, type and protocol version
    serverSocket = socket(AF_INET, gSocketType, gProtocol);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Socket() function failed with error code %d\n", GetLastError());
        WSACleanup();
        return -1;
    }
    else
    {
        printf("Create New socket succesfully!\n");
    }

    // Set socket option
    err = getsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&iOptVal, &iOptLen);
    if (err == SOCKET_ERROR)
    {
        wprintf(L"getsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
    }
    else
        wprintf(L"SO_KEEPALIVE is ON\n");
    err = getsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&iOptVal, &iOptLen);
    if (err == SOCKET_ERROR)
    {
        wprintf(L"getsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
    }
    else
        wprintf(L"SO_REUSEADDR is ON\n");

    // Construct local adress structure
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(PORT);
    serverAdress.sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl(INADDR_LOOPBACK);
    ZeroMemory(&serverAdress.sin_zero, 8);

    // Associate the address information with the socket using bind.
    if (bind(serverSocket, (SOCKADDR *)&serverAdress, sizeof(serverAdress)) == SOCKET_ERROR)
    {
        printf("ERROR: bind failed with error %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // waiting the connection
    if (listen(serverSocket, 255) == SOCKET_ERROR)
    {
        fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    else
    {
        listening = TRUE;
        report(&serverAdress);
        // setHttpHeader(httpHeader);
    }

    // Wait for connection, create and connected socket if a connection is pending
    while (listening)
    {
        Sleep(1);
        clientSocket = accept(serverSocket, (SOCKADDR *)&clientAdress, &sin_size);
        handle_connection(clientSocket, &clientAdress);
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
        // closesocket(serverSocket);
        // reset_connection(serverSocket, serverAdress);
    }

    return 0;
}

void report(SOCKADDR_IN *serverAdress)
{
    // converts a socket address to a corresponding host and service
    char hostBuffer[INET6_ADDRSTRLEN];
    char serviceBuffer[NI_MAXSERV];
    socklen_t addr_len = sizeof(*serverAdress);

    int err = getnameinfo((SOCKADDR *)serverAdress, addr_len, hostBuffer, sizeof(hostBuffer), serviceBuffer, sizeof(serviceBuffer), NI_NUMERICHOST);
    if (err != 0)
    {
        printf("Server not working! \n");
    }
    else
    {
        printf("\n\n\tServer listening on http://%s:%d\n", hostBuffer, serviceBuffer);
    }
}
void handle_connection(SOCKET clientSocket, SOCKADDR_IN *clientAddr_ptr)
{
    unsigned char *ptr, request[500], route[255];
    char *httpHeader;
    int length;
    FILE *file;

    length = recv(clientSocket, request, sizeof(request), 0);
    ptr = strstr(request, "HTTP/"); // Search for valid looking request
    // printf("Got request from %s:%d\t -- request: %s\n", inet_ntoa(clientAddr_ptr->sin_addr), ntohs(clientAddr_ptr->sin_port), request);

    if (ptr == NULL)
        printf("NOT AN HTTP REQUEST!\n");
    else
    {
        *ptr = 0;
        ptr = NULL; // Set pointer to NULL (flag for bad request)

        if (strncmp(request, "GET ", 4) == 0)
        {
            ptr = request + 4; // pointer point now to the URL
            printf("my pointer on GET: %s\n\nmy pointer size: %d\n\n", ptr, strlen(ptr));
            ptr[strlen(ptr) - 1] = '\0';
        }

        if (strncmp(request, "HEAD", 5) == 0)
            ptr = request + 5; // pointer point now to the URL
        if (ptr == NULL)
        {
            // Unrecognized request
            printf("pointer value: %s\n", ptr);
            printf("UNKNOW REQUEST\n");
        }
        else
        {
            // Valid ptr pointing to ressource name or root
            if (ptr[strlen(ptr) - 1] == '/' || ptr[strlen(ptr) - 1] == ' ')
                strcat(ptr, "index.html");

            strcpy(route, WEBROOT);                // begin ressource with webroot path (route = './webroot')
            strcat(route, ptr);                    // Add the ressource requested by client
            printf("Ressource path: %s\n", route); // Check the route

            // Try to open stream for reading
            file = fopen(route, "rb");
            if (file == NULL)
            {
                printf("\t404 Not Found\n");
                int iResult = send(clientSocket, "HTTP/1.0 404 NOT FOUND\r\n<html><head><title>404 Not Found</title></head>", 150, 0);
                if (iResult == SOCKET_ERROR)
                {
                    printf("send failed: %d\n", WSAGetLastError());
                    closesocket(clientSocket);
                    WSACleanup();
                }
            }
            else
            {

                if (ptr == request + 4)
                {
                    // File exist so serve it up
                    printf("Status 200 for ressource: %s\n", route);

                    // Get number of bytes of the ressource requested
                    fseek(file, 0L, SEEK_END);
                    int numbytes = ftell(file);
                    // Reset the file position indicator
                    fseek(file, 0L, SEEK_SET);

                    // Allocate memory for htmlData and HttpHeader
                    ptr = (unsigned char *)malloc(numbytes);

                    // Check for ERROR
                    if (ptr == NULL || httpHeader == NULL)
                    {
                        printf("allocation error..\n");
                        fclose(file);
                        closesocket(clientSocket);
                        WSACleanup();
                    }

                    // Copy file into buffer
                    fread(ptr, sizeof(char), numbytes, file);
                    fclose(file);

                    // Send Body
                    int iResult = send(clientSocket, ptr, numbytes, 0);
                    if (iResult == SOCKET_ERROR)
                    {
                        printf("send failed: %d\n", WSAGetLastError());
                        closesocket(clientSocket);
                        WSACleanup();
                    }
                    else
                    {
                        free(ptr);
                        fclose(file);
                    }
                }
            }
        }
    }
}