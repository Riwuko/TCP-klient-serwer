#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <winsock.h>
#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SIZE 1024

char* SERVER;
const char PORT[] = "1234";

char receiving_data[SIZE];
char temporary_bufor[SIZE];
char sending_data[SIZE];
const char LOAD_COMMAND[]="0,";
const char SHUTDOWN_COMMAND[] = "1,";
const char END[]="end";

using std::string;
using std::cout;

//FUNKCJA WYSYLAJACA SYGNAL KONCA POLACZENIA
void finish_connection_signal(int fd){

    strcpy(sending_data,END);
    printf("Data to server : %s\n\n", sending_data);
    send(fd, sending_data, strlen(sending_data) + 1, 0);
}

//FUNKCJA WYSYLAJACA ŻĄDANIE LISTY DOSTĘPNYCH URZĄDZEŃ
void load_available_devices(int fd){

   strcpy(sending_data, LOAD_COMMAND);
   printf("Data to server : %s\n\n", sending_data);
   send(fd, sending_data, strlen(sending_data) + 1, 0);

   recv(fd, temporary_bufor, SIZE, 0);
   printf("Data from server : %s\n", temporary_bufor);

   recv(fd, receiving_data, SIZE, 0);
   printf("Data from server : %s\n", receiving_data);
}

//FUNKCJA WYSYŁAJĄCA LISTĘ URZĄDZEŃ DO ZAMKNIĘCIA
void shutdown_devices(int fd, char* devices){

   strcpy(sending_data, SHUTDOWN_COMMAND);
   send(fd, sending_data, strlen(sending_data)+1, 0);
   printf("Data to server : %s\n\n", sending_data);

   recv(fd, temporary_bufor, SIZE, 0);
   printf("Data from server : %s\n", temporary_bufor);

   strcpy(sending_data,devices);
   send(fd, sending_data, strlen(sending_data)+1, 0);
   printf("Data to server : %s\n\n", sending_data);

   recv(fd, receiving_data, SIZE, 0);
   printf("Data from server : %s\n", receiving_data);

   load_available_devices(fd);
}

//FUNKCJA NAWIĄZANIA POŁĄCZENIA Z SERWEREM
int establish_server_connection()
{
    //WSADATA SETUP - SPRAWDZENIE CZY BIBLIOTEKA DLA GNIAZD DZIALA POPRAWNIE
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);

    int  err = WSAStartup(wVersionRequested, &wsaData);
    if (err!=0) return -1;

    //CONNECTION SETUP - USTANOWIENIE POLACZENIA Z SERWEREM
    struct sockaddr_in addr;
    struct hostent *host;
    int fd;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(PORT));
    host = gethostbyname(SERVER);
    fd = socket(PF_INET, SOCK_STREAM, 0);

    if (fd < 0)
   {
      fprintf(stderr, "Socket creation error\n");
   }

    memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);
    int connected = connect(fd, (struct sockaddr*)&addr, sizeof(addr));

      if (connected < 0)
   {
      fprintf(stderr, "Server connect error\n");
      return -1;
   }
      return fd;
}

//FUNKCJA ZAKOŃCZENIA POŁĄCZENIA Z SERWEREM
void finish_server_connection(int fd){
    finish_connection_signal(fd);
}

//FUNKCJA OBSŁUGUJĄCA POLECENIA WYWOŁANE PRZEZ UŻYTKOWNIKA GUI
void handle_command(int command, char* list, char* server){
    SERVER = server;
    int fd=establish_server_connection();
    if (fd!=-1){
        switch(command){
        case 0: load_available_devices(fd);
            break;
        case 1: shutdown_devices(fd, list);
            break;
        default:
            break;
        }

        finish_server_connection(fd);
    }
}

#endif // TCPCONNECTION_H
