
#include "WahlSock.h"

namespace WahlSock {

	Server::Server(int _port, int _maxConnections, short _protocol, std::string _name) {
		port = _port;
		protocol = _protocol;
		name = _name;

		readyForConnections = true;

		maxConnections = _maxConnections;

		clientSockets = new SOCKET[maxConnections];
		clientInfo = new SOCKADDR_IN[maxConnections];
		messageBuffers = new char* [maxConnections];
		messageBufferChars = new char[maxConnections * (MAX_MESSAGE_LENGTH_SERVER + 1)];
		connected = new bool[maxConnections];
		messages = new std::queue<std::string>[maxConnections];
		for (int i = 0; i < maxConnections; ++i) {
			messageBuffers[i] = messageBufferChars + i * (MAX_MESSAGE_LENGTH_SERVER + 1);
			connected[i] = false;
			clientSockets[i] = SOCKET_ERROR;
		}
	}

	Server::~Server() {

		listening = false;
		stopListening();

		for (int i = 0; i < clientCount; ++i) {
			if (connected[i]) {
				disconnect(i);
			}
		}
		

		delete[] clientSockets;
		delete[] clientInfo;
		delete[] messageBuffers;
		delete[] messageBufferChars;
		delete[] connected;
		delete[] messages;
	}

	bool Server::setUpListeningSocket() {

		listeningSocket = socket(protocol, SOCK_STREAM, 0);
		if (listeningSocket == SOCKET_ERROR) {
			return false;
		}

		ZeroMemory(&listeningInfo, sizeof(listeningInfo));
		listeningInfo.sin_family = protocol;
		listeningInfo.sin_port = htons(port);
		listeningInfo.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(listeningSocket, (SOCKADDR*)&listeningInfo, sizeof(listeningInfo));
		return true;
	}

	bool Server::startListening() {
		if (setUpListeningSocket()) {
			bind(listeningSocket, (SOCKADDR*)&listeningInfo, sizeof(listeningInfo));
			listening = true;
			std::thread x(&Server::listenThread, this);
			x.detach();
			return true;
		}
		return false;
	}

	void Server::listenThread() {
		listen(listeningSocket, SOMAXCONN);
		while (listening) {
			SOCKADDR_IN _clientInfo;
			int _clientInfoSize = sizeof(_clientInfo);
			ZeroMemory(&_clientInfo, _clientInfoSize);
			SOCKET client = accept(listeningSocket, (SOCKADDR*)&_clientInfo, &_clientInfoSize);
			if (!establishConnection(client, _clientInfo)) {
				send(client, "Unable to connect at this time...", 34, 0);
				closesocket(client);
			}
		}
	}

	bool Server::establishConnection(SOCKET& client, SOCKADDR_IN& _clientInfo) {
		for (int targetCheck = 0; targetCheck < maxConnections; ++targetCheck) {
			if (!isConnected(targetCheck)) {
				clientSockets[targetCheck] = client;
				clientInfo[targetCheck] = _clientInfo;
				std::thread x(&Server::recvThread, this, targetCheck);
				x.detach();
				++clientCount;
				connected[targetCheck] = true;
				std::cout << "Established!" << std::endl;
				return true;
			}
		}

		return false;
	}

	void Server::recvThread(int _target) {
		int target = _target;
		int length;
		while (connected[target]) {
			ZeroMemory(messageBuffers[target], MAX_MESSAGE_LENGTH_SERVER);
			length = recv(clientSockets[target], messageBuffers[target], MAX_MESSAGE_LENGTH_SERVER, 0);
			if (length > 0) {
				std::string messageIn = messageBuffers[target];
				messages[target].push(messageIn);
			}
			else {
				disconnect(target);
				break;
			}
		}
	}

	void Server::stopListening() {
		listening = false;
	}

	bool Server::disconnect(int target) {
		if (connected[target]) {
			closesocket(clientSockets[target]);
			clientSockets[target] = SOCKET_ERROR;
			connected[target] = false;
			--clientCount;
			return true;
		}

		return false;

	}

	bool Server::isConnected(int target) {
		return connected[target];
	}


	bool Server::sendMessage(std::string message, int target) {
		if (isConnected(target)) {
			send(clientSockets[target], message.c_str(), message.length() + 1, 0);
			return true;
		}
		return false;
	}

	int Server::getNumberOfMessages(int target) {
		if (isConnected(target)) {
			return messages[target].size();
		}
		return -1;
	}

	std::string Server::getNextMessage(int target) {
		if (isConnected(target)) {
			std::string returnStr = messages[target].front();
			messages[target].pop();
			return returnStr;
		}
		return "";
	}

	int Server::getNumberOfConnections() {
		return clientCount;
	}
	
	int Server::getConnections(int*& targets) {
		targets = new int[clientCount];
		int count = 0;
		for (int i = 0; i < maxConnections; ++i) {
			if (isConnected(i)) {
				targets[count] = i;
				++count;
				if (count == clientCount - 1) {
					break;
				}
			}
		}
		return clientCount;
	}


	Device Server::getClientInfo(int target) {
		Device device;
		char buff[INET6_ADDRSTRLEN];
		ZeroMemory(buff, sizeof(buff));
		void* addr = &clientInfo[target].sin_addr;
		inet_ntop(protocol, addr, buff, sizeof(buff));

		device.ip = buff;
		device.port = ntohs(clientInfo[target].sin_port);
		device.protocol = protocol;

		return device;
	}

}