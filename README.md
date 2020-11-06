# EE450-socket-project
How to run:
1. make all
2. make serverA
3. make serverB
4. make mainserver
5. ./client
6. Input query parameters

What I have done in the assignment:
1. boot-up phase -> using UDP socket, communication between mainserver and serverA&B
Because serverA goes first, so I let serverB wait for serverA(while binding server port) and sleep(1) during the interval.
2. query & recommend & reply phase: TCP and UDP communication, the details are same as assignment description.
3. data storagement:
unordered_map<string,unordered_map<int,vector<int>> data;
string -> country name, unorder_map<int,vector<int>> -> <node,node's neighbor list> 
I remove repeat elements from data map.
Implement split() and isNum() methods.
4. dynamic TCP port: using rand() in an available range
To avoid conflict between client processes, I check port number (whether in-use) before TCP connection.
5. fork child process to ensure concurrency

What your code files are and what each one of them does:
serverA.cpp & serverB.cpp:
fetch data from txt files/ receive query parameters and do recommendation
servermain.cpp:
push forward deeper backend service(query&recommendation) to serverA&B / communicate with multiple clients by using fork()
client.cpp:
input window and output displayer

The format of all the messages exchanged:
Every time when message is sent by TCP/UDP socket, it must be char*(C string)
When process data, I use int type for user_id and string(C++ class) for country_name

Idiosyncrasy of your project:
It never fail if inputs are valid and correct.
If not, program will exit with error number.

Reused code:
I wrote code all by myself.
