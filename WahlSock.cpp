// WahlSock.cpp : Defines the entry point for the application.
//
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "irprops.lib")
#include "WahlSock.h"

using namespace std;

namespace WahlSock {

	void init() {
		// Initialize winsock
		WSADATA wsData; // A struct for holding the socket information
		WORD ver = MAKEWORD(2, 2); // Version 2.2

		int wsOk = WSAStartup(ver, &wsData); // Starts up the stuff (Version we're using, Where we are storing our port implementation information)
		if (wsOk != 0) {
			cerr << "Can't initialize winsock!" << endl;
		}
	}

	void shutdown() {
		WSACleanup();
	}

	void printDevice(Device& device) {
		if (device.name != "") {
			std::cout << "Device Name: " << device.name << std::endl;
		}

		switch (device.protocol) {
			case(AF_INET):
				std::cout << "Protocol: IPv4" << std::endl;
				break;
			case(AF_INET6):
				std::cout << "Protocol: IPv6" << std::endl;
				break;
			default:
				std::cout << "Protocol: Unknown" << std::endl;
		}

		if (device.ip != "") {
			std::cout << "IP: " << device.ip << std::endl;
		}

		if (device.port != -1) {
			std::cout << "Port: " << device.port << std::endl;
		}
	}

	int getTotDevicesFromAddrInfo(ADDRINFO* hint) {
		ADDRINFO* p;
		int count = 0;
		for (p = hint; p != nullptr; p = p->ai_next) {
			if (&((sockaddr_in*)p->ai_addr)->sin_addr) {
				count++;
			}
			
		}
		return count;
	}

	Device* getIPFromURL(std::string url, int& N) {

		ADDRINFO hints, * res, * p;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC; // ip4 (AF_NET) or ip6 (AF_NET6)
		hints.ai_socktype = SOCK_STREAM;

		int status = getaddrinfo(url.c_str(), NULL, &hints, &res);
		if (status != 0) {
			std::cout << "Failed to getaddrinfo" << std::endl;
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		}

		// Get number of devices from addrinfo linked list
		N = getTotDevicesFromAddrInfo(res);

		Device* deviceArray = new Device[N];
		char ipstr[INET6_ADDRSTRLEN];
		int count = 0;
		void* addr = nullptr;
		sockaddr_in* ip = nullptr;
		for (p = res; p != NULL; p = p->ai_next) {
			if (&((sockaddr_in*)p->ai_addr)->sin_addr) {
				ip = (sockaddr_in*)p->ai_addr;
				addr = &(ip->sin_addr);

				inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);

				deviceArray[count].ip = ipstr;
				deviceArray[count].protocol = p->ai_family;
				deviceArray[count].port = ntohs(ip->sin_port);
				count++;
			}
		}

		freeaddrinfo(res);
		return deviceArray;
	}

	Device setDevice(std::string ip, int port, short protocol, std::string name) {
		Device device;
		device.ip = ip;
		device.port = port;
		device.protocol = protocol;
		device.name = name;
		return device;
	}

}
