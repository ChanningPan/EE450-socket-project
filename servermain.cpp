#include <unordered_map>
#include <string>
#include <sys/select.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
static unordered_map<string,int> country_backend_mapping;

#define SERVER_A_PORT 30242
#define SERVER_B_PORT 31242
#define MAIN_SERVER_UDP 32242
#define MAIN_SERVER_TCP 33242
#define BUFF_SIZE 1024

#define LOCAL_IP_ADDRESS "127.0.0.1"

static vector<string> country_list_A;
static vector<string> country_list_B;
static char* query;
static int query_size;
static string message;

vector<string> split(const string& inputString, const string& splitString)
{
    vector<string> stringVec = {};
    size_t pos = 0;
    size_t inputStringLength = inputString.length();
    size_t splitStringLen = splitString.length();
    if (0 == splitStringLen)
    {
        return stringVec;
    }

    while (pos < inputStringLength)
    {
        int findPos = inputString.find(splitString, pos);
        if (findPos == string::npos)
        {
            string str = inputString.substr(pos, inputStringLength - pos);
            if (str.length() > 0)
            {
                stringVec.emplace_back(str);
            }
            break;
        }

        string elemString = inputString.substr(pos, findPos - pos);
        if (elemString.length() > 0)
        {
            stringVec.emplace_back(elemString);
        }
        pos = findPos + splitStringLen;
    }

    return stringVec;
}

int send_signal(int backend_server)
{
    int port_in;
    if(backend_server == 0)
    {
        port_in = SERVER_A_PORT;
    }
    else
    {
        port_in = SERVER_B_PORT;
    }

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // set address and port
    struct sockaddr_in addr;
    socklen_t addr_len=sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;       // Use IPV4
    addr.sin_port  = htons(port_in);  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // time out
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 200000;  // 200 ms
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));

    addr.sin_family = AF_INET;
    addr.sin_port  = htons(MAIN_SERVER_UDP);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sendto(sockfd, "Good", 4, 0, (sockaddr*)&addr, addr_len);
    close(sockfd);
    return 0;
}

int receive_country_list(int backend_server)
{
    int port_out;
    if(backend_server == 0)
    {
        port_out = SERVER_A_PORT;
    }
    else
    {
        port_out = SERVER_B_PORT;
    }

    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    socklen_t addr_len=sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET; 
    addr.sin_port   = htons(port_out);    
    addr.sin_addr.s_addr = htonl(INADDR_ANY);


    if (bind(sockfd, (struct sockaddr*)&addr, addr_len) == -1)
    {
        printf("Failed to bind socket on port %d\n", port_out);
        close(sockfd);
        return -1;
    }

    char buffer[BUFF_SIZE];
    memset(buffer, 0, BUFF_SIZE);

    while(1)
    {
        send_signal(backend_server);
        struct sockaddr_in src;
        socklen_t src_len = sizeof(src);
        memset(&src, 0, sizeof(src));
        int sz = recvfrom(sockfd, buffer, BUFF_SIZE, 0, (sockaddr*)&src, &src_len);
        if (sz > 0)
        {
            buffer[sz] = 0;
            string countries = buffer;
            if(backend_server == 0)
            {
                country_list_A = split(countries,"+");
            }
            else
            {
                country_list_B = split(countries,"+");
            }
	        break;        
	    }
    }
    close(sockfd);
    return 0;
}

int display_country_list()
{
    cout << "ServerA" << "             |" << "ServerB" << endl;
    for(int i=0; i < max(country_list_A.size(),country_list_B.size());i++)
    {
        if(i < country_list_A.size())
        {
            cout << country_list_A[i];
            for (int j=0; j < 20-country_list_A[i].length(); j++)
            {
                cout << " ";
            }
        cout << "|";
        }
        else
        {
            cout << "                  |";
        }
        if(i < country_list_B.size())
        {
            cout << country_list_B[i];
        }
        cout << endl;
    }
    return 0;
}

int send_query(int backend_server, char* query, int size)
{
    int port_in;
    if(backend_server == 0)
    {
        port_in = SERVER_A_PORT;
    }
    else
    {
        port_in = SERVER_B_PORT;
    }

    int sock_fd;  
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);  
        
    struct sockaddr_in addr_serv;  
    int len;  
    memset(&addr_serv, 0, sizeof(addr_serv));  
    addr_serv.sin_family = AF_INET;  
    addr_serv.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);  
    addr_serv.sin_port = htons(port_in);  
    len = sizeof(addr_serv);  
    
    int recv_num;   
    char recv_buf[20];  
        
    sendto(sock_fd, query, size, 0, (struct sockaddr *)&addr_serv, len);
    while(1)
    {
        recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);  
        if(recv_num > 0)  
        {  
            recv_buf[recv_num] = '\0';  
            message = recv_buf;
            close(sock_fd);
            break;
        }  
    }
    close(sock_fd);
}

int send_message(int client_port, string message)
{
    int   sockfd, n;
    struct sockaddr_in  servaddr;

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(client_port);

    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("server connect error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    if( send(sockfd, message.c_str(), message.length(), 0) < 0){
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    close(sockfd);
}

int main()
{
    printf("The Main server is up and running.\n");
    receive_country_list(0);
    printf("The Main server has received the country list from serverA using UDP over port %d\n",MAIN_SERVER_UDP);
    sleep(1);
    receive_country_list(1);
    printf("The Main server has received the country list from serverB using UDP over port %d\n",MAIN_SERVER_UDP);
    display_country_list();
    // Add country-server mapping relationship
    for(int i=0; i < country_list_A.size();i++)
    {
        country_backend_mapping[country_list_A[i]] = 0;
    }
    for(int i=0; i < country_list_B.size();i++)
    {
        country_backend_mapping[country_list_B[i]] = 1;
    }

    // receive query
    string country;
    int user_id;
    
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
    servaddr.sin_port = htons(MAIN_SERVER_TCP);

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    if(listen(listenfd, 10) == -1){
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    int client_port;

    while(1)
    {
        if((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1)
        {
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            close(listenfd);
            exit(-5);
        }
        
        pid_t id = fork();
        if(id == 0){
            close(listenfd);
            while(1)
            {
                n = recv(connfd, buff, 4096, 0);
                if (n > 0)
                {
                    buff[n] = '\0';
                    query = buff;
                    query_size = n;
                    close(connfd);
                    string tmp = buff;
                    vector<string> args = split(tmp,"+");
                    country = args[0];
                    user_id = stoi(args[1]);
                    client_port = stoi(args[2]);
                    printf("The Main server has received the request on User %d in %s from client using TCP over port %d\n",user_id,country.c_str(),MAIN_SERVER_TCP);
                    break;
                }
            }
            // recognize backend server
            bool find_res = false;
            for(int i=0; i< country_list_A.size();i++)
            {
                if(country_list_A[i] == country)
                {
                    find_res = true;
                    break;
                }
            }
            for(int i=0; i< country_list_B.size();i++)
            {
                if(country_list_B[i] == country)
                {
                    find_res = true;
                    break;
                }
            }

            if(!find_res)
            {
                message = country+" not found";
                printf("%s does not show up in server A&B\n",country.c_str());
                send_message(client_port,message);
                printf("The Main Server has sent “%s” to client using TCP over port %d\n", message.c_str(), MAIN_SERVER_TCP);
            }
            else
            {
                int backend_server = country_backend_mapping[country];
                string server_name;
                if(backend_server == 0)
                {
                    server_name = "A";
                }
                else
                {
                    server_name = "B";
                }
                
                printf("%s shows up in server %s\n",country.c_str(),server_name.c_str());

                send_query(backend_server,query,query_size);
                printf("The Main Server has sent request from User %d to server %s using UDP over port %d\n",user_id,server_name.c_str(),MAIN_SERVER_UDP);
                
                if(message == "not found")
                {
                    message = "User " + to_string(user_id)+ " not found";
                    printf("The Main server has received “%s” from server %s\n",message.c_str(),server_name.c_str());
                    send_message(client_port,message);
                    printf("The Main Server has sent error to client using TCP over %d\n",MAIN_SERVER_TCP);
                }
                else if(message == "-1")
                {
                    message = "User None is possible friend of User " + to_string(user_id) + " in "+ country;
                    printf("The Main server has received searching result(s) of User %d from server%s\n",user_id,server_name.c_str());
                    send_message(client_port,message);
                    printf("The Main Server has sent searching result(s) to client using TCP over port %d\n",MAIN_SERVER_TCP);
                }
                else
                {
                    message = "User "+ message + " is possible friend of User " + to_string(user_id) + " in "+ country;
                    printf("The Main server has received searching result(s) of User %d from server%s\n",user_id,server_name.c_str());
                    send_message(client_port,message);
                    printf("The Main Server has sent searching result(s) to client using TCP over port %d\n",MAIN_SERVER_TCP);
                }
            }
            exit(0);
        }
        else
        {
            close(connfd);
        }
    }
    close(listenfd);
    return 0;
}