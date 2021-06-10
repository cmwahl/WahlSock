
#include "WahlSock.h"

#include "stdlib.h"
#include "stdio.h"

int mainClientTest(int argc, char* argv[]);
int mainServerTest(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	mainClientTest(argc, argv);
	return 0;
}

int mainClientTest(int argc, char* argv[]) {
	WahlSock::init();

	WahlSock::Device server = WahlSock::setDevice("127.0.0.1", 80);
	WahlSock::Client client(server);

	std::cout << "Attempting to connect..." << std::endl;
	std::cout << "Connected: " << client.connectToServer() << std::endl;;

	while (!client.getNumberOfMessages()) {

	}

	std::cout << "Received: " << client.getNextMessage() << std::endl;

	client.sendMessage("What's up homIEEE?");

	client.disconnect();

	WahlSock::shutdown();
	
	return 0;
}


int mainServerTest(int argc, char* argv[]) {

	WahlSock::init();
	WahlSock::Device* devices = nullptr;
	
	std::string ip = "127.0.0.1";
	int port = 80;
	
	WahlSock::Server server(port, 3, AF_INET);
	std::cout << "Listening? " << server.startListening() << std::endl;;

	std::cout << "Waiting..." << std::endl;
	while (!server.getNumberOfConnections()) {

	}

	//server.stopListening();
	//server.sendMessage("Connected...", 0);
	//WahlSock::printDevice(server.getClientInfo(0));
	while (true) {
		for (int i = 0; i < 3; ++i) {
			if (server.isConnected(i) && server.getNumberOfMessages(i)) {
				std::cout << "Received: " << server.getNextMessage(i) << std::endl;
				server.sendMessage("Received!", i);
			}
		}
		if (!(server.isConnected(0) || server.isConnected(1) || server.isConnected(2))) {
			break;
		}

	}
	server.stopListening();
	delete[] devices;
	WahlSock::shutdown();
	std::cout << "We done" << std::endl;
	return 0;
}


