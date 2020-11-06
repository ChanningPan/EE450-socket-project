#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include<arpa/inet.h>
#include<unistd.h>

using namespace std;

#define SERVER_PORT 33242
#define LOCAL_IP_ADDRESS "127.0.0.1"

int get_result(int port)
{
    int  listenfd, connfd;
    struct sockaddr_in  servaddr;
    char  buff[4096];
    int  n;

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    if(listen(listenfd, 10) == -1){
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    while(1){
        if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            continue;
        }
        n = recv(connfd, buff, 4096, 0);
        if (n > 0)
        {
            buff[n] = '\0';
            cout << buff << endl;
            break;
        }
        close(connfd);
    }
    close(listenfd);
}

int detect_port(int port)
{
    int detectfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  detectaddr;
    memset(&detectaddr, 0, sizeof(detectaddr));
    detectaddr.sin_family = AF_INET;
    detectaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    detectaddr.sin_port = htons(port);
    int res = bind(detectfd, (struct sockaddr*)&detectaddr, sizeof(detectaddr));
    close(detectfd);
    return res;
}

int main(){
    printf("Client is up and running\n");
    while(1)
    {
        string country_name;
        int user_id;
        cout << "Enter country name: ";
        cin >> country_name;
        cout << "Enter user ID: ";
        cin >> user_id;
        
        // dynamic TCP port number, and detect whether port is in-use
        int port = rand()%10 + 49152;
        while(detect_port(port) == -1)
        {
            port = rand()%10000 + 49152;
        }

        // send query
        int   sockfd, n;
        string sendline = country_name+"+"+to_string(user_id)+"+"+to_string(port);
        struct sockaddr_in  servaddr;

        if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
            return 0;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SERVER_PORT);

        if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
            printf("client connect error: %s(errno: %d)\n",strerror(errno),errno);
            return 0;
        }

        if( send(sockfd, sendline.c_str(), sendline.length(), 0) < 0){
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            return 0;
        }
        close(sockfd);
        printf("Client has sent User%d and %s to Main Server using TCP\n",user_id,country_name.c_str());
        
        // wait for final result
        get_result(port);
        cout << endl << "-----Start a new request-----" << endl;
    }

}