#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <map>
#include <iostream>
#include <assert.h>
#include <tuple>
using namespace std;
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

typedef long long LL;

#define pb push_back
#define debug(x) cout << #x << " : " << x << endl
#define part cout << "-----------------------------------" << endl;

///////////////////////////////
#define MAX_CLIENTS 4
#define PORT_ARG 8001

const int initial_msg_len = 256;
pthread_t thread_pool[100];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
map<int, string> dict;
int x=0,m=0;

typedef struct queue
{
    struct queue *next;
    int *client_socket;
} queue;

queue *head = NULL;
queue *tail = NULL;

const LL buff_sz = 1048576;

void insert_into_queue(int *client_socket)
{
    queue *newnode = (queue *)malloc(sizeof(queue));
    newnode->client_socket = client_socket;
    newnode->next = NULL;
    if (tail == NULL)
    {
        head = newnode;
    }
    else
    {
        tail->next = newnode;
    }
    tail = newnode;
}

int *delete_from_queue()
{
    if (head == NULL)
    {
        return NULL;
    }
    else
    {
        int *result = head->client_socket;
        queue *temp = head;
        head = head->next;
        if (head == NULL)
        {
            tail = NULL;
        }
        free(temp);
        return result;
    }
}
pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. \n";
    }

    output[bytes_received] = 0;
    output.resize(bytes_received);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}

///////////////////////////////

void *handle_connection(void *client_socket_fd)
{
    x++;
    int client_socket = *((int *)client_socket_fd);
    int received_num, sent_num;
    int ret_val = 1;
    string cmd;
    tie(cmd, received_num) = read_string_from_socket(client_socket, buff_sz);
    ret_val = received_num;
    if (ret_val <= 0)
    {
        printf("Server could not read msg sent from client\n");
        goto close_client_socket_ceremony;
    }
    pthread_mutex_unlock(&mutex1);
    if (cmd[1] == 'i')
    {
        int key = cmd[7] - 48;
        string r = cmd.substr(8, cmd.length() - 8);
        auto itr = dict.find(key);
        string msg_to_send_back;
        if (itr != dict.end())
        {
            msg_to_send_back = "if";
        }
        else
        {
           // cout << key << " " << r << " " << endl;
            dict[key] = r;
            msg_to_send_back = "insert";
        }
        pthread_mutex_unlock(&mutex1);
        sleep(2);
        int sent_to_client = send_string_on_socket(client_socket, msg_to_send_back);
    }
    if (cmd[1] == 'd')
    {
       // printf("Hello\n");
       int key = cmd[7]-48;
       // printf("%d\n",key);
        auto itr = dict.find(key);
        string msg_to_send_back;
        if (itr == dict.end())
        {
            msg_to_send_back = "df";
        }
        else
        {
            dict.erase(key);
            msg_to_send_back = "delete";
        }
        pthread_mutex_unlock(&mutex1);
        sleep(2);
        int sent_to_client = send_string_on_socket(client_socket, msg_to_send_back);
    }
    if(cmd[1] == 'u')
    {
        int key = cmd[7]-48;
        auto itr = dict.find(key);
        string r = cmd.substr(8, cmd.length() - 8);
        string msg_to_send_back;
        if (itr == dict.end())
        {
            msg_to_send_back = "u";
            msg_to_send_back += "\n";
        }
        else
        {
             dict[key] = r;
             msg_to_send_back = dict[key];
            
        }
        pthread_mutex_unlock(&mutex1);
        sleep(2);
        int sent_to_client = send_string_on_socket(client_socket, msg_to_send_back);
    }

    if(cmd[1] == 'f')
    {
        int key = cmd[5]-48;
        auto itr = dict.find(key);
        string msg_to_send_back;
        if (itr == dict.end())
        {
            msg_to_send_back = "ff";
        }
        else
        {
             msg_to_send_back = key;
 
        }
        pthread_mutex_unlock(&mutex1);
        sleep(2);
        int sent_to_client = send_string_on_socket(client_socket, msg_to_send_back);
    }
    if(cmd[1] == 'c')
    {
        int key1 = cmd[7]-48;
        int key2 = cmd[8]-48;
        auto itr1 = dict.find(key1);
        auto itr2 = dict.find(key2);
        string msg_to_send_back;
        if (itr1 == dict.end() || itr2 == dict.end())
        {
            msg_to_send_back = "c";
            msg_to_send_back += "\n";
        }
        else
        {
             string s = dict[key1];
             dict[key1] = dict[key1] + dict[key2];
             dict[key2] = dict[key2] + s;
             msg_to_send_back = dict[key2];
            
        }
        pthread_mutex_unlock(&mutex1);
        sleep(2);
        int sent_to_client = send_string_on_socket(client_socket, msg_to_send_back);
    }

close_client_socket_ceremony:
    if(x==m)
    {
    close(client_socket);
    }
  //  printf(BRED "Disconnected from client" ANSI_RESET "\n");
    return NULL;
}

void *func(void *args)
{
    while (true)
    {
        pthread_mutex_lock(&mutex1);
        int *request_client = delete_from_queue();
        pthread_mutex_unlock(&mutex1);
        if (request_client != NULL)
        {
            handle_connection(request_client);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{

    int i, j, k, t, n;

    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;

    struct sockaddr_in serv_addr_obj, client_addr_obj;
    //cout<<argc<<endl;
    n = atoi(argv[1]);
    m = n;
    //printf("%d\n",n);
    for (int i = 0; i < n; i++)
    {
        pthread_create(&thread_pool[i], NULL, func, NULL);
    }
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); //process specifies port
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);

    while (1)
    {
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        int *client_req;
        client_req = (int *)malloc(sizeof(int));
        *client_req = client_socket_fd;
        pthread_mutex_lock(&mutex1);
        //printf("Hi\n");
        insert_into_queue(client_req);
        pthread_mutex_unlock(&mutex1);
    }

    close(wel_socket_fd);
    return 0;
}