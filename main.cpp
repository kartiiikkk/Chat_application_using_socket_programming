#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

/*
	//initialize winsock library

	//create the socket
	
	//get ip and port

	//bind the ip/port with the socket

	//listen on the socket

	//accept (isme call ruki rahegi jb tk bahar se kisi client ka call nhi aaega)

	//receive and send

	//close the socket

	//clean the winsock



*/

bool Initialize() {

	WSADATA data; //contains the windows socket implementation information

	return WSAStartup(MAKEWORD(2, 2), &data) == 0;

}

void InteractWithClient(SOCKET clientSocket,vector<SOCKET>& clients) {
	//send/recv client //ab send and receive idhr se karenge taki main thread free ho jaye to take another connection

	char buffer[4096]; //used to store the data from the client

	while (1) {
		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0); //in here in recv it will be blocked until it gets the data

		if (bytesrecvd <= 0) {

			cout << "client Disconnected" << endl;

			break;

		}

		string message(buffer, bytesrecvd);
		cout << "message from client:" << message << endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}
	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end()) {
		clients.erase(it); 
	}

	closesocket(clientSocket);
}

int main() {

	//initialization of library

	if (!Initialize()) {
		cout << "winsock initialization failed" << endl;
		return 1;
	}

	cout << "server" << endl;
	//Socket Creation
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (listenSocket == INVALID_SOCKET) {
		cout << "socket creation failed" << endl;
		return 1;
	}

	//create address structure
	int port = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	//convert ipaddress (0.0.0.0) put it inside sin_family int the binary format
	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) { //API(InetPton)
		
		cout << "setting address structure failed" << endl;
		closesocket(listenSocket);
		
		WSACleanup();

		return 1;
	}

	//bind the ip/port with the socket
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "bind failed" << endl;
		closesocket(listenSocket);

		WSACleanup();

		return 1; 
	}

	//listen
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) { //here somaxconn give the maximum user on the server queue
		cout << "listen failed" << endl;
		closesocket(listenSocket);

		WSACleanup();

		return 1;
	}
	cout << "server has started listening on port:"<< port << endl; 
	vector<SOCKET> clients;

	while (1) {
		//start accepting the connection from the client
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr); //now this closeSocket is used for a specific client that it has connected now and listen socket is used for connecting to the different clients

		//accept takes one client and give the further work to recv and then it waits for the upcoming client

		if (clientSocket == INVALID_SOCKET) { //if this doesn't happen then we are connected to the client
			cout << "invalid client socket" << endl;
		}

		clients.push_back(clientSocket);

		thread t1(InteractWithClient, clientSocket, std::ref(clients));
		t1.detach();
	}

	closesocket(listenSocket);

	WSACleanup();
	return 0;
}