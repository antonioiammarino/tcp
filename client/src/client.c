#if defined WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "protocol.h"

void errorhandler(char *error_message)
{
	printf("%s\n", error_message);
}

void clearWinsock()
{
	#if defined WIN32
		WSACleanup();
	#endif
}

int main(int argc, char *argv[])
{
	#if defined WIN32
		WSADATA wsa_data;
		int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (result != 0)
		{
			errorhandler("Error at WSAStartup() \n");
			return -1;
		}
	#endif

		//client socket and connection
		int c_socket;
		c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (c_socket < 0)
		{
			errorhandler("Socket creation failed. \n");
			system("pause");
			closesocket(c_socket);
			clearWinsock();
			return -1;
		}

		struct sockaddr_in sad;
		memset(&sad, 0, sizeof(sad));
		sad.sin_family = AF_INET;
		sad.sin_addr.s_addr = inet_addr("127.0.0.1");
		sad.sin_port = htons(PROTOPORT);

		if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0)
		{
			errorhandler("Failed to connect.\n");
			system("pause");
			closesocket(c_socket);
			clearWinsock();
			return -1;
		}

		//"Connected!" from server
		int bytes;
		char buffer[BUFFER_SIZE];
		memset(buffer, '\0', BUFFER_SIZE);
		if ((bytes = recv(c_socket, buffer, BUFFER_SIZE-1, 0)) <= 0)
		{
			errorhandler("recv() failed or connection closed prematurely");
			system("pause");
			closesocket(c_socket);
			clearWinsock();
			return -1;
		}
		printf("%s\n", buffer);

		char operation[BUFFER_SIZE], operation_result[BUFFER_SIZE];
		memset(operation, '\0', BUFFER_SIZE);
		memset(operation_result, '\0', BUFFER_SIZE);
		int lenght;
		puts("Enter the operation like this ----> operator[space]number[space]number[Enter] (Es. + 23 45)");
		do
		{
			puts("Press Enter only after the operation");
			gets(operation);	//gets reads the whole line including spaces
		}while(strcmp(operation, "")==0); //while string is empty

		lenght=strlen(operation);
		if (send(c_socket, operation, lenght, 0) != lenght)
		{
			errorhandler("send() sent a different number of bytes than expected");
			system("pause");
			closesocket(c_socket);
			clearWinsock();
			return -1;
		}

		while(strcmp(operation, "=")!=0)	//strcmp returns 0 if operation is 0. = ends the client inputs
		{
			if ((bytes = recv(c_socket, operation_result, BUFFER_SIZE-1, 0)) <= 0)
			{
				errorhandler("recv() failed or connection closed prematurely");
				system("pause");
				closesocket(c_socket);
				clearWinsock();
				return -1;
			}
			printf("%s\n\n", operation_result);	//print the result
			memset(operation_result, '\0', BUFFER_SIZE);

			puts("Enter a new operation");
			memset(operation, '\0', BUFFER_SIZE);
			do
			{
				puts("Press Enter only after the operation");
				gets(operation);//gets reads the whole line including spaces
			}while(strcmp(operation, "")==0); //while string is empty

			lenght=strlen(operation);
			if (send(c_socket, operation, lenght, 0) != lenght)
			{
				errorhandler("send() sent a different number of bytes than expected");
				system("pause");
				closesocket(c_socket);
				clearWinsock();
				return -1;
			}
		}
		system("pause");
		closesocket(c_socket);
		clearWinsock();
		return 0;
}
