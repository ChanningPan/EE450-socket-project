#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream> 
#include <arpa/inet.h>
#include<sys/select.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <set>
#include <algorithm>

using namespace std;

#define LOCAL_IP_ADDRESS "127.0.0.1"
#define SERVER_A_PORT 30242
#define MAIN_SERVER_UDP 32242
#define BUFF_SIZE 1024

static unordered_map<string,unordered_map<int,vector<int>>> data;
static string query_country;
static int query_user_id;

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

bool isNum(string str)  
{  
	stringstream sin(str);  
	double d;  
	char c;  
	if(!(sin >> d))  
	{
		return false;
	}
	if (sin >> c) 
	{
		return false;
	}  
	return true;  
}

vector<string> loadData()
{
    vector<string> country_list;
    string line;   
    ifstream datafile ("data1.txt");
    string curr_country = "";

    while (getline (datafile, line))
    { 
        vector<string> res = split(line," ");
        if(!isNum(res[0]))
        {
            curr_country = res[0];
            country_list.push_back(curr_country);
            unordered_map<int,vector<int>> G;
            data[curr_country] = G; 
        }
        else{
            int curr_node = stoi(res[0]);
            for(int i=1; i < res.size();i++)
            {
                if(res[i] == res[0]){
                    continue;
                }
                data[curr_country][curr_node].push_back(stoi(res[i]));
            }
        }
    }
    datafile.close();
    return country_list;
}

string list_to_str(vector<string> list)
{
    string str = "";
    for(int i=0; i<list.size(); i++)
    {
        str += list[i];
        if(i < list.size()-1)
        {
            str += "+";
        }
    }
    return str;
}

int first_wait()
{
    int port_out = MAIN_SERVER_UDP;

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
        struct sockaddr_in src;
        socklen_t src_len = sizeof(src);
        memset(&src, 0, sizeof(src));
        int sz = recvfrom(sockfd, buffer, BUFF_SIZE, 0, (sockaddr*)&src, &src_len);

        if (sz > 0)
        {
            buffer[sz] = 0;
	        break;        
	    }
    }
    close(sockfd);
    return 0;    
}

int send_country_list(string countries)
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // set address and port
    struct sockaddr_in addr;
    socklen_t addr_len=sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port  = htons(MAIN_SERVER_UDP);  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // time out
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 200000;  // 200 ms
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
    
    addr.sin_family = AF_INET;
    addr.sin_port  = htons(SERVER_A_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sendto(sockfd, countries.c_str(), countries.length(), 0, (sockaddr*)&addr, addr_len);
    sleep(1);

    close(sockfd);
    return 0;
}

int recommendation(string country, int user_id)
{
    unordered_map<int,vector<int>> graph = data[country];
    set<int> neighbors;
    for(int i=0; i< graph[user_id].size();i++)
    {
        neighbors.insert(graph[user_id][i]);
    } 
    vector<int> nodes;
    for(auto kv : graph) {
        if(kv.first == user_id)
        {
            continue;
        }
        if(neighbors.count(kv.first) == 0)
        {
            nodes.push_back(kv.first);
        }
    }
    int res = -1;
    if(nodes.size()==0)
    {
        return res;
    }
    int common_num = 0;
    int max_degree = 0;
    int max_degree_id = -1;

    for(int i=0; i< nodes.size(); i++)
    {
        if(graph[nodes[i]].size() > max_degree)
        {
            max_degree = graph[nodes[i]].size();
            max_degree_id = nodes[i];
        }
        else if(graph[nodes[i]].size() > max_degree)
        {
            if(nodes[i] < max_degree_id)
            {
                max_degree_id = nodes[i];
            }
        }
        int curr = 0;
        for(int j = 0; j< graph[nodes[i]].size();j++)
        {
            if(neighbors.count(graph[nodes[i]][j]) == 1)
            {
                curr++;
            }
        }
        if(curr > common_num)
        {
            res = nodes[i];
            common_num = curr;
        }
        else if(curr == common_num)
        {
            if(common_num == 0)
            {
                res = max_degree_id;
            }
            else
            {
                if(res > nodes[i])
                {
                    res = nodes[i];
                }
            }
        }
    }
    printf("Here are the results: ");
    if(res >=0)
    {
        printf("User %s\n",to_string(res).c_str());
    }
    return res;
}

int wait_for_query()
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);    
    struct sockaddr_in addr_serv;  
    int len;  
    memset(&addr_serv, 0, sizeof(struct sockaddr_in));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(SERVER_A_PORT);
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY); 
    len = sizeof(addr_serv);  
    if(bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0)  
    {  
        perror("bind error:");  
        exit(1);  
    }  

        
    int recv_num;   
    char recv_buf[20];  
    struct sockaddr_in addr_client;  

    while(1)  
    {   
        recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);  
        if(recv_num > 0)
        {
            recv_buf[recv_num] = '\0';
            string tmp = recv_buf;
            vector<string> args = split(tmp,"+");
            query_country = args[0];
            query_user_id = stoi(args[1]);
            printf("The server A has received request for finding possible friends of User%d in %s\n",query_user_id,query_country.c_str());
            break;
        }
    }
    
    unordered_map<int,vector<int>> graph = data[query_country];
    vector<int> nodes;
    for(auto kv : graph) {
        nodes.push_back(kv.first);
    }
    bool search_res = false;
    for(int i=0; i<nodes.size(); i++)
    {
        if(nodes[i] == query_user_id)
        {
            search_res = true;
            break;
        }
    }
    if(!search_res)
    {
        printf("User%d does not show up in %s\n",query_user_id,query_country.c_str());
        sendto(sock_fd, "not found", 9, 0, (struct sockaddr *)&addr_client, len);
        printf("The server A has sent “User");
        printf("%d",query_user_id);
        printf(" not found” to Main Server\n");
    }
    else
    {
        printf("The server A is searching possible friends for User %s ...\n",to_string(query_user_id).c_str());
        int recommend_res = recommendation(query_country,query_user_id);
        string res_str = to_string(recommend_res);
        int size = res_str.length();
        sendto(sock_fd, res_str.c_str(), size, 0, (struct sockaddr *)&addr_client, len);
        printf("The server A has sent the result(s) to Main Server");
    }
    close(sock_fd);
    return 0;    
}

int remove_repeat()
{
    for(auto kv: data)
    {
        for(auto kvv:kv.second)
        {
            vector<int> v = kvv.second;
            sort(v.begin(),v.end());
            v.erase(unique(v.begin(), v.end()), v.end());
            data[kv.first][kvv.first] = v;
        }
    }
    return 0;
}

int main()
{
    printf("The server A is up and running using UDP on port %d\n",SERVER_A_PORT);
    vector<string> country_list = loadData();
    remove_repeat();
    first_wait();
    send_country_list(list_to_str(country_list));
    printf("The server A has sent a country list to Main Server\n");
    while(1)
    {
        wait_for_query();
    }
}