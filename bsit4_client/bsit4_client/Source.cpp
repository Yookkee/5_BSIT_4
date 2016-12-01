#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>

#define PRINT_ERROR(a, b) \
	std::cout << "-----Error-----" << std::endl; \
	std::cout << a << " " << " : " << WSAGetLastError() << std::endl; \
	std::cout << a << " " << " : " << GetLastError() << std::endl;

#define CLEAN_UP() \
if (client_socket) closesocket(client_socket); \
	WSACleanup();

enum COMMANDS {HELP, EXIT};

int main_activity(SOCKET client_socket);

int do_help();

int main()
{
	int status;
	SOCKET client_socket = NULL;
	//SOCKET server_socket = NULL;

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

	/* Connect */
	std::string str;
	std::cout << "Enter server IP address: "; // Просим ввести ip-адрес сервера
	std::cin >> str; // Получаемего

	sockaddr_in ServerAddress;
	ServerAddress.sin_family = AF_INET; // TCP-IP соедение
	ServerAddress.sin_port = htons(16789); // Порт
	ServerAddress.sin_addr.s_addr = inet_addr(str.c_str()); //Преобразуемадресвнужныйформат
	if (connect(client_socket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{ /*Ошибка. Пояснение можно получить функцией  WSAGetLastError */
		PRINT_ERROR("main", "connect");
		CLEAN_UP();
		return 1;
	}
	std::cout << "Connection established..." << std::endl;

	status = main_activity(client_socket);

	CLEAN_UP();
	return 0;

}

int main_activity(SOCKET client_socket)
{
	do_help();
	
	while (true)
	{
		int cmd;
		std::cin >> cmd;

		switch (cmd)
		{
		case COMMANDS::HELP:
			do_help();
		case COMMANDS::EXIT:
			return 0;
		}
	}
}

int do_help()
{
	char help[] =
		"help - get this message\n\
		quit - close the program";

	std::cout << help << std::endl;
}