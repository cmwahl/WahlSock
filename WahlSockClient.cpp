
#include "WahlSock.h"

namespace WahlSock {

	Client::Client(std::string _ip, int _port, short _protocol, std::string _name) {
		server.ip = _ip;
		server.port = _port;
		server.protocol = _protocol;
		server.name = _name;

		setServerInfo();
		readyToConnect = createClientSocket();
		connected = false;
	}

	Client::Client(Device& _server) {
		server = _server;

		setServerInfo();
		readyToConnect = createClientSocket();
		connected = false;
	}

	bool Client::createClientSocket() {
		clientSocket = socket(server.protocol, SOCK_STREAM, 0);
		if (clientSocket == SOCKET_ERROR) {
			std::cout << "Failed to create client socket" << std::endl;
			return false;
		}
		return true;
	}

	void Client::setServerInfo() {
		SOCKADDR_IN* serverInfoPointer = (SOCKADDR_IN*)&serverInfo;
		serverInfoPointer->sin_family = server.protocol;
		serverInfoPointer->sin_port = htons(server.port);
		inet_pton(AF_INET, server.ip.c_str(), &serverInfoPointer->sin_addr);
	}

	Client::~Client() {
		disconnect();
	}

	bool Client::connectToServer() {
		if (connect(clientSocket, &serverInfo, sizeof(serverInfo))) {
			return false;
		}
		connected = true;
		startReceiving();
		return true;
	}

	bool Client::isReadyToConnect() {
		return readyToConnect;
	}

	bool Client::isConnected() {
		return connected;
	}

	void Client::startReceiving() {
		std::thread receiver(&Client::recvThread, this);
		receiver.detach();
	}

	void Client::recvThread() {
		while (connected) {
			ZeroMemory(messageBuffer, MAX_MESSAGE_LENGTH_CLIENT);
			int bytesReceived = recv(clientSocket, messageBuffer, MAX_MESSAGE_LENGTH_CLIENT, 0);
			if (bytesReceived <= 0) {
				disconnect();
				break;
			}
			std::string message(messageBuffer);
			messages.push(messageBuffer);
		}
	}

	bool Client::sendMessage(std::string message) {
		if (connected) {
			send(clientSocket, message.c_str(), message.length() + 1, 0);
			return true;
		}
		return false;
	}

	int Client::getNumberOfMessages() {
		return messages.size();
	}

	std::string Client::getNextMessage() {
		std::string toReturn = messages.front();
		messages.pop();
		return toReturn;
	}

	bool Client::newServer(Device& device) {
		server = device;
		readyToConnect = createClientSocket();
		return readyToConnect;
	}

	bool Client::disconnect() {
		if (connected) {
			closesocket(clientSocket);
			connected = false;
			return true;
		}
		return false;
	}

}