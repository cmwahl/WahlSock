// WahlSock.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <string>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <queue>

#ifdef MAX_MESSAGE_LENGTH_CLIENT
#else
#define MAX_MESSAGE_LENGTH_CLIENT 1024
#endif

#ifdef MAX_MESSAGE_LENGTH_SERVER
#else
#define MAX_MESSAGE_LENGTH_SERVER 1024
#endif

namespace WahlSock {
	void init();
	void shutdown();

	struct Device {
		std::string ip = "";
		char protocol = -1;
		int port = 0;
		std::string name = "";
	};

	void printDevice(Device& device);
	Device setDevice(std::string _ip, int _port, short _protocol = AF_INET, std::string name = "");
	Device* getIPFromURL(std::string url, int& N);

	class Client {
	public:
		Client(std::string _ip, int _port, short _protocol = AF_INET, std::string _name = "");
		Client(Device& _server);
		~Client();

		bool connectToServer();
		bool isReadyToConnect();
		bool isConnected();

		bool sendMessage(std::string message);
		int getNumberOfMessages();
		std::string getNextMessage();

		bool disconnect();

		bool newServer(Device& server);

	private:
		SOCKET clientSocket;
		bool createClientSocket();

		Device server;
		void setServerInfo();
		SOCKADDR serverInfo;
		int serverInfoLength = sizeof(serverInfo);

		bool readyToConnect;
		bool connected;

		void startReceiving();
		std::queue<std::string> messages;
		char messageBuffer[MAX_MESSAGE_LENGTH_CLIENT + 1];

		void recvThread();

	};

	class Server {
	public:

		Server(int _port, int _connections = 1, short _protocol = AF_INET, std::string _name = "");
		~Server();

		bool startListening();
		void stopListening();

		int getNumberOfConnections();
		int getConnections(int*& targets);
		bool isConnected(int target);

		bool sendMessage(std::string message, int target);

		std::string getNextMessage(int target);
		int getNumberOfMessages(int target);

		bool disconnect(int target);

		Device getClientInfo(int target);

	private:

		std::string name = "";
		int port = 0;
		short protocol = 0;
		bool readyForConnections;
		bool listening;
		int maxConnections;

		SOCKET listeningSocket;
		SOCKADDR_IN listeningInfo;

		int clientCount = 0;
		SOCKET* clientSockets = nullptr;
		SOCKADDR_IN* clientInfo = nullptr;
		char** messageBuffers = nullptr;
		char* messageBufferChars = nullptr;
		bool* connected = nullptr;
		std::queue<std::string>* messages = nullptr;

		void recvThread(int target);
		void listenThread();

		bool setUpListeningSocket();
		bool establishConnection(SOCKET& client, SOCKADDR_IN& _clientInfo);




	};
}