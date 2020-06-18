#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//declare important information
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

//declare libaries
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#pragma comment (lib, "Ws2_32.lib")

//declare namespace
using namespace std;

//initialize the socket
WSADATA wsaData;

//declare variables
string gameinfo;
int recvbuflen = DEFAULT_BUFLEN;
char* connectioninfo;
string ip;

//declare file system functionality
ifstream infoFile;

//DWORD WINAPI menu() {
//	int menuchoice = 0;
//	if (menuchoice == 0) {
//		cout << endl << "Currently connected to server." << endl << "1) Disconnect" << endl << endl << " > ";
//		int menuoption = 0;
//		cin >> menuoption;
//		if (menuoption == 1) {
//			cout << "disconnecting";
//		}
//		else {
//			menuchoice = 0;
//		}
//	}
//	return 0;
//}

//main proceedure
int main(int argc, char* argv[]) {
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed: " << iResult;
		return 1;
	}
	else {
		cout << "Client started" << endl;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	//prepare socket for connection
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//check for address information
	//if ip wasnt specified on command arguments
	if (argv[1] == NULL) {
		//try and open serverconfig file
		infoFile.open("serverConfig.txt");
		//if the serverconfig file fails...
		if (infoFile.fail()) {
			cout << "No connection information file was found and no IP was specified" << endl;
			WSACleanup();
			return 1;
		}
		//else if the serverconfig file exists...
		else {
			cout << "Using connection information file" << endl;
			getline(infoFile, ip);
			connectioninfo = &ip[0];
		}
	}
	else {
		//else use the IP address in command arguments...
		cout << "Using specified IP, ignoring connection info file" << endl;
		connectioninfo = argv[1];
	}

	iResult = getaddrinfo(connectioninfo, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	//start socket
	SOCKET ConnectSocket = INVALID_SOCKET;

	ptr = result;
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << "Error at socket: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	//connect to server
	iResult = connect(ConnectSocket, ptr->ai_addr,(int)ptr->ai_addrlen);
	cout << "Attempting connection to " << connectioninfo << " on port " << DEFAULT_PORT << "..." << endl;
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		return 1;
	}
	else {
		cout << "Connected to server successfully" << endl << "Awaiting information from server..." << endl;
		int receivesize;
		int gamedatarecieved = 0;
		int menuactive = 0;
		while (1 == 1) {
			char serverresponse[500];
			memset(serverresponse, '\0', 500);
			recv(ConnectSocket, serverresponse, 500, NULL);
			//if server is full...
			if (strstr(serverresponse, "SERVERERRORFULL")) {
				cout << "Server terminated connection - Server full"; // TO BE REPLACED WITH SPECTATOR SYSTEM
				WSACleanup();
				exit(0);
			}
			else if (strstr(serverresponse, "HEARTBEAT")) {
				cout << "Recieved heartbeat request from server - Responding" << endl;
				char heartbeatresponse[18] = "HEARTBEATRESPONSE";
				send(ConnectSocket, heartbeatresponse, strlen(heartbeatresponse), NULL);
			}
			//get game info
			else if (gamedatarecieved == 0) {
				gameinfo = serverresponse;
				cout << "RECIEVED GAME INFO: " << gameinfo << endl;
				gamedatarecieved++;
			}
			//if (menuactive == 0) {
			//	menuactive = 1;
			//	thread mainmenu(menu);
			//}
			//wait for minimum clients
			if (strstr(serverresponse, "WAITINGFORMIN")) {
				cout << "Server is waiting for minimum clients to be reached before game starts\n";
			}
			//if connection to server is lost...
			if ((receivesize = recv(ConnectSocket, serverresponse, 500, NULL)) == SOCKET_ERROR) {
				cout << "Connection to the server was lost.";
				WSACleanup();
				exit(0);
			}
		}
	}
}
	