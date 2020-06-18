#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//declare important information
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

//declare libaries
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <time.h>
#pragma comment (lib, "Ws2_32.lib")

//declare namespace
using namespace std;

//initialize the socket
WSADATA wsaData;
string ipaddress;

//declare variables
int connectedclients = 0;
int minclients = 0;
string gamename;
string mapname;
string difficultyname;
char receivebuffer[DEFAULT_BUFLEN];
int receivebufferlength = DEFAULT_BUFLEN;
int receivebufferbytes;

//declare filesystem functionality
ofstream infoFile;
ofstream logFile;

//handle data recieved from clients
bool incomingpackethandler(SOCKET ClientSocket) {
	//receive packets
	receivebufferbytes = recv(ClientSocket, receivebuffer, receivebufferlength, NULL);
	if (receivebufferbytes > 0) {
		if (receivebuffer == "HEARTBEATRESPONSE") {
			cout << "Client responded to heartbeat request." << endl;
		}
		receivebufferbytes = 0;
	}
	return 0;
}

//create file with connection information
int generateInfoFile() {
	infoFile.open("serverConfig.txt");
	cout << "IP: " << ipaddress << endl;
	cout << "PORT: " << DEFAULT_PORT << endl << endl;
	infoFile << ipaddress + "\n" + DEFAULT_PORT;
	infoFile.close();
	logFile << "IP: " << ipaddress << endl << "Port: " << DEFAULT_PORT << endl;
	return 0;
}

//start game
int gamestart() {
	cout << "[SERVER]: " << gamename << " on " << mapname << " in progress" << endl << endl;
	logFile << gamename << " on " << mapname << " in progress" << endl;
	return 0;
}

//thread to handle client sockets
DWORD WINAPI clientthreadhandler(void* ClientSocket) {
	int threadactive = true;
	SOCKET socket = (SOCKET)ClientSocket;
	cout << "[SERVER]: Created thread for connected client" << endl;
	logFile << "Created thread for connected client" << endl;

	//sendgameinfo
	char gameinfo[500];
	string gamestring = gamename + ", " + mapname + ", " + difficultyname;
	strcpy_s(gameinfo, gamestring.c_str());
	send(socket, gameinfo, strlen(gameinfo), NULL);
	cout << "[SERVER]: Sent game info to client" << endl << endl;
	logFile << "Sent game info to client" << endl;

	if (connectedclients < minclients) {
		cout << "[SERVER]: Waiting for more clients before game can start" << endl << endl;
		logFile << "Waiting for more clients before game can start" << endl;
		char waitingmessage[14] = "WAITINGFORMIN";
		send(socket, waitingmessage, strlen(waitingmessage), NULL);
	}
	else if (connectedclients == minclients) {
		cout << "[SERVER]: Minimum clients reached, starting game" << endl;
		logFile << "Minimum clients reached, starting game" << endl;
		gamestart(); // game start proceedure
	}

	while (threadactive == true) {
		//listen for incoming info
		incomingpackethandler(socket);
	}
	return 0;
}

//main proceedure
int main() {
	int addressinfo, sendresult;
	logFile.open("logFile.txt", 1);

	//set gamemode
	int gamemode = 0;
	cout << "Please select game type:" << endl << endl << "     1) Deathmatch" << endl << "     2) Capture the Flag" << endl << "     3) Blood Diamond" << endl << endl << "> ";
	while (gamemode == 0) {
		cin >> gamemode;
		if (gamemode == 1) {
			gamename = "Deathmatch";
		}
		else if (gamemode == 2) {
			gamename = "Capture the Flag";
		}
		else if (gamemode == 3) {
			gamename = "Blood Diamond";
		}
		else {
			gamemode = 0;
			cout << "Invalid entry" << endl << endl << "> ";
		}
	}
	cout << "Gamemode selected is " << gamename << endl;
	logFile << "Set gamemode to " << gamename << endl;

	//set map
	int mapnumber = 0;
	cout << endl << "Please select map:" << endl << endl << "    1) Martian Engineering Compound" << endl << "     2) Alpha Dam" << endl << "     3) Derelict Tunnels " << endl << endl << "> ";
	while (mapnumber == 0) {
		cin >> mapnumber;
		if (mapnumber == 1) {
			mapname = "Martian Engineering Compound";
		}
		else if (mapnumber == 2) {
			mapname = "Alpha Dam";
		}
		else if (mapnumber == 3) {
			mapname = "Derelict Tunnels";
		}
		else {
			mapnumber = 0;
			cout << "Invalid entry" << endl << endl << "> ";
		}
	}
	cout << "Map selected is " << mapname << endl;
	logFile << "Set map to " << mapname << endl;

	//set difficulty
	int difficultynumber = 0;
	cout << endl << "Please select difficulty level:" << endl << endl <<"     1) Easy" << endl << "     2) Normal" << endl << "     3) Hard" << endl << "     4) Extreme" << endl << endl << "> ";
	while (difficultynumber == 0) {
		cin >> difficultynumber;
		if (difficultynumber == 1) {
			difficultyname = "Easy";
		}
		else if (difficultynumber == 2) {
			difficultyname = "Normal";
		}
		else if (difficultynumber == 3) {
			difficultyname = "Hard";
		}
		else if (difficultynumber == 4) {
			difficultyname = "Extreme";
		}
		else {
			mapnumber = 0;
			cout << "Invalid entry" << endl << endl << "> ";
		}
	}
	cout << "Difficulty selected is " << difficultyname << endl << endl;
	logFile << "Set difficulty to " << difficultyname << endl;

	//set maxclients
	int maxclients = 0;
	while (maxclients == 0) {
		cout << "Please enter the maximum number of players: ";
		cin >> maxclients;
		if (maxclients <= 1) {
			maxclients = 0;
			cout << "Maximum clients cannot be lower than 2." << endl;
		}
	}
	cout << "Maximum clients set to " << maxclients << endl << endl;
	logFile << "Set max clients to " << maxclients << endl;

	//set minclients
	while (minclients == 0) {
		cout << "Please enter the minimum number of players: ";
		cin >> minclients;
		if (minclients <= 1) {
			minclients = 0;
			cout << "Minimum clients cannot be lower than 2." << endl;
		}
		else if (minclients > maxclients) {
			minclients = 0;
			cout << "Minimum clients cannot be greater than maximum clients." << endl;
		}
	}
	cout << "Minimum clients set to " << minclients << endl;
	logFile << "Set min clients to " << minclients << endl;

	//prepare for main socket
	addressinfo = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (addressinfo != 0) {
		cout << "[SERVER]: WSAStartup failed: " << addressinfo << endl;
		logFile << "WSAStartup failed: " << addressinfo << endl;
		return 1;
	}
	else {
		cout << endl << "Gamemode set to " << difficultyname << " difficulty " << gamename << " on " << mapname << endl;
		cout << "Maximum clients is " << maxclients << ", minimum clients for game to start is " << minclients << endl << endl << endl << "~~~ SERVER STARTING ~~~" << endl << endl << endl;
		cout << "[SERVER]: Starting TCP connectivity..." << endl;
		logFile << "Starting TCP connectivity..." << endl;
	}

	//setup main socket for connections
	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	SOCKADDR_IN clientinformation = { 0 };

	addressinfo = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (addressinfo != 0) {
		cout << "[SERVER]: Address information capture failed: " << addressinfo << endl;
		logFile << "Address information capture failed: " << addressinfo << endl;
		WSACleanup();
		return 1;	
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		cout << "[SERVER]: Error at socket: " << WSAGetLastError() << endl;
		logFile << "Error at socket: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	addressinfo = bind(ListenSocket, result->ai_addr,
		(int)result->ai_addrlen);
	if (addressinfo == SOCKET_ERROR) {
		cout << "[SERVER]: Address bind failed with error: " << WSAGetLastError() << endl;
		logFile << "Address bind failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		closesocket(ListenSocket); WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	//start listening for connections
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "[SERVER]: Listening error: " << WSAGetLastError() << endl;
		logFile << "Listening error: " << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else {
		//get ip address of server
		char buffer[INET_ADDRSTRLEN];

		char ac[80];
		if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
			cerr << "Error " << WSAGetLastError() << " when getting local host name." << endl;
			return 1;
		}
		struct hostent *phe = gethostbyname(ac);
		if (phe == 0) {
			return 1;
		}

		for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
			ipaddress = inet_ntoa(addr);
		}
		
		cout << "[SERVER]: Now listening for connections" << endl << endl;
		logFile << "Now listening for connections" << endl;
		generateInfoFile();
	}

	//client socket handler
	SOCKET ClientSocket;
	while (1 == 1) {

		ClientSocket = INVALID_SOCKET;
		ClientSocket = accept(ListenSocket, (struct sockaddr*)&clientinformation, NULL);

		if (ClientSocket == INVALID_SOCKET) {
			cout << "[SERVER]: Accept failed: " << WSAGetLastError() << endl;
			logFile << "Accept failed: " << WSAGetLastError() << endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		else {
			if (connectedclients < maxclients) {

				cout << "[CONNECTION]: Accepted client" << endl;
				logFile << "Accepted client" << endl;
				connectedclients++;
				cout << "[PLAYERCOUNT]: Connected clients: " << connectedclients << endl;
				logFile << "Connected clients: " << connectedclients << endl;
				CreateThread(0, 0, clientthreadhandler, (void*)ClientSocket, 0, 0);
			}
			else {
				cout << "[CONNECTION] Rejecting client connection attempt - no space available on server" << endl;
				logFile << "Rejecting client connection attempt - no space available on server";
				//send disconnect alert
				char disconnectmessage[500] = "SERVERERRORFULL";
				send(ClientSocket, disconnectmessage, strlen(disconnectmessage), NULL);
				//close socket (TO BE REPLACED WITH THE SPECTATOR SYSTEM)
				closesocket(ClientSocket);
			}
		}
	}
}