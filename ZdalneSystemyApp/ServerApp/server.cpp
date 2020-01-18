#include "sshfunctions.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <libssh/libssh.h>
#include <pthread.h>


#define SIZE 1024
#define MAX_DEVICES 24

using std::string;
using std::cout;

//char* SHUTDOWN = "sudo shutdown now";
char* SHUTDOWN = "eject";
char* ADDRESS_SCHEMAT = "student@lab-net-";
char* LOAD = "load";
const char* ACCEPT = "accept";
const char* DECLINE = "ERR"; 


struct client {
    int cfd;
    struct sockaddr_in caddr;
};

//funkcja laczaca sie po SSH z danym adresem i wykonujaca wskazana komende
int execute_command(string address, char* weak_command){
    
    char device_address[address.length()+1];
    strcpy(device_address, address.c_str());
    
    ssh_session remote_session = ssh_new();
    ssh_options_set(remote_session, SSH_OPTIONS_HOST, device_address);
    int available = ssh_connect(remote_session);
    if (weak_command==LOAD){
        if(available != SSH_OK){
            return -1;
        } else return 0;
    }
    char* password;
    std::size_t found = address.find("riwuko");
    if(found!=std::string::npos)
        password = "serdelki12";
    found = address.find("zurek");
    if(found!=std::string::npos)
        password = "xC%?5q3iA!H=\\s#";
    found = address.find("student");
    if(found!=std::string::npos)
        password = "student";
    
    string string_password(password);
    string string_command(weak_command);
    string strong_command = "echo '"+string_password+"' | sudo -S "+string_command;
    char command[strong_command.size() + 1]; 
    strcpy(command, strong_command.c_str());
    
    int rc=ssh_userauth_password(remote_session, NULL,password); 
    if (rc!=SSH_AUTH_SUCCESS){
      printf("Authentication failed \n");   
      return -1;
    }
    
    show_remote_processes(remote_session, command, password);
    
    ssh_free(remote_session);
    return 0;
}

//funkcja wczytujaca ktore urzadzenia sa dostepne w sieci
void handle_load(int cfd){
    string available_devices;
    string device_address[3] = {"riwuko@192.168.1.18","zurek@192.168.1.19"};
    for (int i=0; i<2; i++){
        //string device_address = ADDRESS_SCHEMAT + std::to_string(i+1);
        //string device_address = "localhost";
        
        int is_available = execute_command(device_address[i], LOAD);
        if (is_available==0){
            //available_devices = available_devices + ADDRESS_SCHEMAT + std::to_string(i+1) + ",";
        available_devices = available_devices + device_address[i] + ",";
            
        }
    }
    
    cout<<"Available devices: "<<available_devices<<"\n";
        
    if(available_devices.length()!=0){
        char cstr[available_devices.length()+1];
        strcpy(cstr, available_devices.c_str());
        write(cfd, cstr, sizeof(cstr));
    }else{
        write(cfd,DECLINE,sizeof(DECLINE));
    }
}

//funkcja odbierajaca adresy urzadzen do zamkniecia
void handle_shutdown(int cfd){
    char buf[SIZE];
    memset(buf, 0, SIZE);
    
    int received_count = read(cfd, buf, SIZE);
    cout<<buf<<"\n\n";
    
    string device_number;
    string device_address;
    for (int i=0;i<strlen(buf);i++){
        char tmp = buf[i];
        char separate=',';
        if(tmp!=separate){
            device_number =  device_number + tmp;

        }
        if (tmp==separate or ((i+1)==strlen(buf))){
            device_address=device_number;
            cout<<"Device for shutdown: \n" << device_address << "\n\n";
            execute_command(device_address, SHUTDOWN);
        
            device_number = "";
        }
    }
    write(cfd, ACCEPT, strlen(ACCEPT)+1);
}

int client_connection(int cfd){
    char buf[SIZE];
    memset(buf, 0, SIZE);
    
    int received_count = read(cfd, buf, SIZE);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Command: %s\n",buf);
    write(cfd, ACCEPT, strlen(ACCEPT)+1);
    
    if((received_count = strcmp(buf, "1,")) == 0) {
        memset(buf, 0, SIZE);
        handle_shutdown(cfd);
    }
    
    else if((received_count = strcmp(buf, "0,")) == 0){
        memset(buf, 0, SIZE);
        handle_load(cfd);
    }
    else return 1;
    
    if(strcmp (buf,"end")==0)
        return 1;
    
    return 0;
}

void* thread_routine(void*arg) {

    struct client* my_client = (struct client*)arg;

    printf("\n\n|||||||\nNew connection from: %s\n: ", inet_ntoa((struct in_addr)my_client->caddr.sin_addr));

            while(1){
                
                int finish = client_connection(my_client->cfd);
                if (finish) break;
                
            }
        close(my_client->cfd);
        printf("\nConnection closed\n|||||||\n\n");
        free(my_client);
        return EXIT_SUCCESS;  
    
}

int main(int argc, char *argv[]) {

    struct sockaddr_in saddr, caddr;
    int fd, cfd, on = 1;
    pthread_t thread;
    
    fd = socket(PF_INET, SOCK_STREAM, 0);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(1234);
    saddr.sin_addr.s_addr = INADDR_ANY;
    
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    
    bind(fd, (struct sockaddr*) & saddr, sizeof(saddr));
    listen(fd, 10);
    
        // buf=1 : SHUTDOWN COMMAND
        // buf=0 : LOAD AVAILABLE DEVICES COMMAND

    while(1) {
    
        struct client* new_client = static_cast<struct client*>(malloc(sizeof(struct client)));
        socklen_t length = sizeof(new_client->caddr);
        new_client->cfd = accept(fd, (struct sockaddr*)&new_client->caddr, &length);
        pthread_create(&thread, NULL, thread_routine, new_client);
        pthread_detach(thread);
    }

    close(fd);

    return EXIT_SUCCESS;
}
