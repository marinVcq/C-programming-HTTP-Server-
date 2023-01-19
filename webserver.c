#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define DEFAULT_PORT 80
#define WEBROOT "./webroot"
#define MAX_BUFFER_SIZE 1024
#define BACKLOG 10

void setHttpHeader(char httpHeader[])
{
    // File object to return to the client
    FILE *htmlData = fopen("index.html", "r");

    char line[100];
    char responseData[8000];
    while (fgets(line, sizeof(line), htmlData) != NULL)
    {
        strcat(responseData, line);
    }
    strcat(httpHeader, responseData);
}

void handle_connection(SOCKET socket, SOCKADDR_IN *clientAddr);

void report(SOCKADDR_IN *serverAdress);

int main(void)
{
    WSADATA wsaData;
    SOCKET serverSocket;
    SOCKET clientSocket;
    SOCKADDR_IN serverAdress, clientAdress;
    WORD wVersionRequested = MAKEWORD(2, 2);
    BOOL Continue = TRUE;
    BOOL listening = FALSE;

    char httpHeader[8000] = "HTTP/1.0 200 OK\r\n\n";

    int gAdresseFamily = AF_UNSPEC,
        gSocketType = SOCK_STREAM,
        gProtocol = IPPROTO_TCP;

    int err;

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

    // Construct local adress structure
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(DEFAULT_PORT);
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
    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    else
    {
        listening = TRUE;
        report(&serverAdress);
        setHttpHeader(httpHeader);
    }

    // Wait for connection, create and connected socket if a connection is pending
    while (listening)
    {
        socklen_t sin_size = sizeof(SOCKADDR_IN);
        clientSocket = accept(serverSocket, (SOCKADDR *)&clientAdress, &sin_size);
        handle_connection(clientSocket, &clientAdress);
        // printf("Record connection\n");
        closesocket(clientSocket);
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
    unsigned char *ptr, request[500], ressource[255];
    int length;
    FILE *file;

    length = recv(clientSocket, request, sizeof(request), 0);

    printf("Got request from %s:%d\t -- request: %s\n", inet_ntoa(clientAddr_ptr->sin_addr), ntohs(clientAddr_ptr->sin_port), request);

    ptr = strstr(request, " HTTP/"); // Search for valid looking request

    if (ptr == NULL)
    {
        printf("NOT AN HTTP REQUEST!\n");
    }
    else
    {
        *ptr = 0;
        ptr = NULL; // Set pointer to NULL (flag for bad request)
        printf("THE REQUEST IS: %s \n", request);
        if (strncmp(request, "GET ", 4) == 0)
        { // It's a GET request
            printf("THIS IS A GET REQUEST   \n");
            ptr = request + 4; // pointer point now to the URL
        }
        if (strcmp(request, "HEAD") == 0)
        { // Its an HEAD request
            printf("THIS IS A GET REQUEST   \n");
            ptr = request + 5; // pointer point now to the URL
        }
        if (ptr == NULL)
        {
            // Unrecognized request
            printf("pointer value: %s\n", ptr);
            printf("UNKNOW REQUEST\n");
        }
        else
        {
            // Handle Valid request
            if (ptr[strlen(ptr) - 1] == '/')
            {
                // add the index file
                strcat(ptr, "index.html");
                strcpy(ressource, WEBROOT); // begin ressource with webroot path
                strcat(ressource, ptr);
                printf("ressouce path: %s\n", ressource);
                file = fopen(ressource, "r");
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
                    printf("\t\nOpening the ressource: %s\n", ressource);
                    printf(" 200 OK\n");
                    if (ptr == request + 4)
                    {
                        // get the number of bytes of the ressource
                        fseek(file, 0L, SEEK_END);
                        int numbytes = ftell(file);

                        // Reset the file position indicator
                        fseek(file, 0L, SEEK_SET);

                        // Allocate memory for the buffer char pointer
                        ptr = (char *)calloc(numbytes, sizeof(char));
                        // check for ERROR
                        if (ptr == NULL)
                        {
                            fclose(file);
                            closesocket(clientSocket);
                            WSACleanup();
                        }

                        // Copy all the file into the buffer
                        fread(ptr, sizeof(char), numbytes, file);
                        fclose(file);

                        // Confirm reading success
                        printf("buffer contains HTML text\n");

                        // Send to client
                        int iResult = send(clientSocket, ptr, (int)strlen(ptr), 0);
                        if (iResult == SOCKET_ERROR)
                        {
                            printf("send failed: %d\n", WSAGetLastError());
                            closesocket(clientSocket);
                            WSACleanup();
                        }
                        else
                        {
                            printf("Send HTML succesfully!\n");
                            printf("HTML content: %s\n", ptr);
                            free(ptr);
                            fclose(file);
                        }
                    }
                    closesocket(clientSocket);
                }
            }
        }
    }
}