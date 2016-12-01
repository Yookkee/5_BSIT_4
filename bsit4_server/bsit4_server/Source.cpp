#include <winsock2.h>
#include <iostream>
#include "info_collector.h"
#include "protocol.h"
#include <Aclapi.h>

#pragma comment(lib, "ws2_32.lib")

#define PRINT_ERROR(a, b) \
	std::cout << "-----Error-----" << std::endl; \
	std::cout << a << " " << " : " << WSAGetLastError() << std::endl; \
	std::cout << a << " " << " : " << GetLastError() << std::endl;

#define CLEAN_UP() \
	if (server_socket) closesocket(server_socket); \
	if (client_socket) closesocket(client_socket); \
	WSACleanup();

int send_message(std::string msg);
std::string recv_message();

int status;
SOCKET client_socket = NULL;
SOCKET server_socket = NULL;

int main_activity();
int do_send_hello();


int main()
{
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
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == INVALID_SOCKET)
	{ 
		/* Socket function failed with */
		PRINT_ERROR("main", "socket");
		return 1;
	}

	/* Socket binding */
	sockaddr_in addr; //структура, содержащая адрес сокета и информацию о нем
	addr.sin_family = AF_INET; // Наш протокол
	addr.sin_port = htons(16789); // 16789 – выбранный для взаимодействия порт
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY - вселокальныеинтерфейсы

	status = bind(server_socket, (LPSOCKADDR)&addr, sizeof(addr));
	if (status == SOCKET_ERROR) {
		//wprintf(L"bind failed with error %u\n", WSAGetLastError());
		PRINT_ERROR("main", "bind");
		CLEAN_UP();
		return 1;
	}

	/* Listening */
	if (listen(server_socket, 2) == SOCKET_ERROR)
	{/*Ошибка. Пояснение можно получить функцией  WSAGetLastError().*/
		PRINT_ERROR("main", "listen");
		CLEAN_UP();
		return 1;
	}

	//----------------------
	// Accept the connection.
	while (true)
	{
		client_socket = accept(server_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET) {
			PRINT_ERROR("main", "listen");
			CLEAN_UP();
			return 1;
		}
		else
		{
			std::cout << "Client Connected" << std::endl;
		}

		main_activity();
	}


	CLEAN_UP();
	return 0;
}

int main_activity()
{

	/*
	PSID pOwnerSID;
	PSECURITY_DESCRIPTOR pSecDescr;
	int dwRes = GetNamedSecurityInfoA("MACHINE\\HKEY_CURRENT_USER\\SOFTWARE\\Audacity\\Audacity", SE_REGISTRY_KEY, OWNER_SECURITY_INFORMATION, &pOwnerSID, NULL, NULL, NULL, &pSecDescr);
	*/
	char cmd = -1;
	while (true)
	{
		std::string msg;

		msg = recv_message();
		if (msg.length() == 0)
		{
			if (cmd == -1) continue;
			std::cout << "Error : main_activity : msg.lenght == 0" << std::endl;
			return -1;
		}
		cmd = msg.c_str()[0];
		cmd -= REQUEST_BIT;

		switch (cmd)
		{
		case COMMANDS::HELLO:
			do_send_hello();
			std::cout << "Hello" << std::endl;
			break;
		case COMMANDS::VERSION:
			send_message(do_get_os_version());
			std::cout << "OS Version requested" << std::endl;
			break;
		case COMMANDS::TIME:
			send_message(do_get_current_time());
			std::cout << "Time requested" << std::endl;
			break;
		case COMMANDS::TICKS:
			send_message(do_get_ticks());
			std::cout << "Ticks requested" << std::endl;
			break;
		case COMMANDS::MEMORY:
			send_message(do_get_memory_info());
			std::cout << "Memory info requested" << std::endl;
			break;
		case COMMANDS::DISKS:
			send_message(do_get_disks_info());
			std::cout << "Disks info requested" << std::endl;
			break;
		case COMMANDS::OWNER:
			send_message(do_get_owner(msg.c_str() + 1));
			std::cout << "Owner requested" << std::endl;
			break;
		case COMMANDS::ACL_ACE:
			send_message(do_get_acl(msg.c_str() + 1));
			std::cout << "ACL requested" << std::endl;
			break;
		default:
			std::cout << "Unknown Command" << std::endl;
		}
		cmd = 0;
	}
}

int do_send_hello()
{
	std::string msg = "Hello!!!";
	send_message(msg);

	return 0;
}

int send_message(std::string msg)
{
	int iResult = send(client_socket, msg.c_str(), (int)strlen(msg.c_str()), 0);
	if (iResult == SOCKET_ERROR) {
		/*wprintf(L"send failed with error: %d\n", WSAGetLastError());
		closesocket(client_socket);
		WSACleanup();**/

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
	char buf[BUFLEN];

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