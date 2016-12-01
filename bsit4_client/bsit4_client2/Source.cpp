#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include "../../bsit4_server/bsit4_server/protocol.h"

#define PRINT_ERROR(a, b) \
	std::cout << "-----Error-----" << std::endl; \
	std::cout << a << " " << " : " << WSAGetLastError() << std::endl; \
	std::cout << a << " " << " : " << GetLastError() << std::endl;

#define CLEAN_UP() \
if (client_socket) closesocket(client_socket); \
	WSACleanup();

/*
enum COMMANDS { VERSION = 1, HELLO = 95, HELP = 90, QUIT = 99 };
#define REQUEST_BIT 128
#define RESPONSE_BIT 0
*/
int send_message(std::string msg);
std::string recv_message();

int main_activity();
int do_help();
int do_get_hello();
int send_request(int cmd);
int send_request_param(int cmd);

SOCKET client_socket = NULL;

int main()
{
	setlocale(LC_ALL, "Russian");

	int status;

	/* WinSock v1.1 Initialization */
	WSADATA ws;
	status = WSAStartup(MAKEWORD(1, 1), &ws);
	if (status != 0) {
		/* Tell the user that we could not find a usable Winsock DLL. */
		std::cout << "-----Error-----" << std::endl;
		std::cout << "main WSAStartup : " << status << std::endl;
		return 1;
	}

	/* Create Socket */
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET)
	{
		/* Socket function failed with */
		PRINT_ERROR("main", "socket");
		return 1;
	}

	std::string addr;
	std::cout << "Enter server IP address: "; // Просим ввести ip-адрес сервера
	std::cin >> addr; // Получаемего
	//char addr[] = "127.0.0.1";

	sockaddr_in ServerAddress;
	ServerAddress.sin_family = AF_INET; // TCP-IP соедение
	ServerAddress.sin_port = htons(16789); // Порт
	ServerAddress.sin_addr.s_addr = inet_addr(addr.c_str()); //Преобразуемадресвнужныйформат
	if (connect(client_socket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{ /*Ошибка. Пояснение можно получить функцией  WSAGetLastError */
		PRINT_ERROR("main", "connect");
		CLEAN_UP();
		return 1;
	}
	std::cout << "Connection established." << std::endl;

	status = main_activity();
	CLEAN_UP();
	return status;
}

int main_activity()
{
	do_help();

	while (true)
	{
		int cmd;
		std::cout << ">>> ";
		std::cin >> cmd;

		switch (cmd)
		{
		case COMMANDS::HELP:
			do_help();
			break;
		case COMMANDS::QUIT:
			return 0;
		case COMMANDS::HELLO:
			do_get_hello();
			break;
		case COMMANDS::OWNER:
			send_request_param(cmd);
			break;
		case COMMANDS::ACL_ACE:
			send_request_param(cmd);
			break;
		default:
			if (cmd >= COMMANDS::VERSION && cmd <= COMMANDS::TIME)
				send_request(cmd);
			else
				std::cout << "Unknown Command" << std::endl;
		}
	}
}

int do_help()
{
	std::cout << COMMANDS::VERSION << " - get os version" << std::endl;
	std::cout << COMMANDS::TICKS << " - get ticks since os start" << std::endl;
	std::cout << COMMANDS::MEMORY << " - get memory info" << std::endl;
	std::cout << COMMANDS::DISKS << " - get disks info" << std::endl;
	std::cout << COMMANDS::OWNER << " - get owner info" << std::endl;
	std::cout << COMMANDS::ACL_ACE << " - get ace" << std::endl;
	std::cout << COMMANDS::TIME << " - get current date and time" << std::endl;

	std::cout << COMMANDS::HELP << " - help message" << std::endl;
	std::cout << COMMANDS::HELLO << " - get hello message from server" << std::endl;
	std::cout << COMMANDS::QUIT << " - exit" << std::endl;

	return 0;
}

int send_request(int cmd)
{
	std::string msg = "";
	BYTE b = REQUEST_BIT;
	b += cmd;
	msg += b;

	send_message(msg);
	msg = recv_message();

	std::cout << msg << std::endl;
	return 0;
}

int do_get_hello()
{
	std::string msg = "";
	BYTE b = REQUEST_BIT;
	b += COMMANDS::HELLO;
	msg += b;

	send_message(msg);
	msg = recv_message();

	std::cout << msg << std::endl;
	return 0;
}

int send_message(std::string msg)
{
	int iResult = send(client_socket, msg.c_str(), (int)strlen(msg.c_str()), 0);
	if (iResult == SOCKET_ERROR) {
		// wprintf(L"send failed with error: %d\n", WSAGetLastError());
		// closesocket(client_socket);
		// WSACleanup();

		PRINT_ERROR("send_message", "send");
		CLEAN_UP();

		return 1;
	}

}

std::string recv_message()
{
#define BUFLEN 256

	std::string msg = "";
	int iResult;
	char buf[BUFLEN + 1];

	do {
		iResult = recv(client_socket, buf, BUFLEN, 0);
		if (iResult > 0)
		{
			buf[iResult] = 0;
			msg += buf;
		}
		//wprintf(L"Connection closed\n");
		else
		{
			PRINT_ERROR("recv_message", "recv");
			return "";
		}

	} while (iResult == BUFLEN);

#undef BUFLEN

	return msg;
}


int send_request_param(int cmd)
{
	std::string msg = "";
	BYTE b = REQUEST_BIT;
	b += cmd;
	msg += b;

	std::cout << "Enter path : ";
	std::string path;
	bool need_fix = false;
	if (path[0] == '\"') need_fix = true;

	int x = -1;
	do{
		if (x != -1)
			msg += ' ';

		std::cin >> path;
		msg += path;
	} while ((x = std::cin.rdbuf()->in_avail()) > 1);

	if (need_fix)
	{
		msg.pop_back();
		msg = msg.c_str() + 1;
	}

	send_message(msg);
	msg = recv_message();

	std::cout << msg << std::endl;
	return 0;

}