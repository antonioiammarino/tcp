#if defined WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "protocol.h"

void clearWinsock()
{
	#if defined WIN32
	WSACleanup();
	#endif
}

void errorhandler(char* errorMessage)
{
	printf("%s\n", errorMessage);
}

//This function send the operation result to the client
void send_result(char* string, int client_socket, int my_socket)
{
	int lenght = strlen(string);
	if (send(client_socket, string, lenght, 0) != lenght)
	{
		printf("client-send() sent a different number of bytes than expected");
		system("pause");
		closesocket(my_socket);
		clearWinsock();
	}
}

//This function send an error message to the client if the client input is wrong
void send_error(int client_socket, int my_socket)
{
	char* string="Incorrect formatting or unknown operator";
	int lenght = strlen(string);
	if (send(client_socket, string, lenght, 0) != lenght)
	{
		printf("client-send() sent a different number of bytes than expected");
		system("pause");
		closesocket(my_socket);
		clearWinsock();
	}
}

/* This function checks the correct number format:
 * number can't contain letters or characters other than digits;
 * number cant't be NULL */
int check_number(char* number)
{
	int error=0;
	if(number==NULL)
	{
		error++;
	}
	else
	{
		for(unsigned int i=0; i<strlen(number); i++)
		{
			if(isalnum(number[i]))
			{
				if(isalpha(number[i]))
				{
					error++;
					break;
				}
			}
			else
			{
				error++;
				break;
			}
		}
	}
	if(number[0]=='-')
		error--;

	if(error!=0)
		return 0;
	else
		return 1;
}

// This function divides string
int token(char* item, int* num1, int* num2)
{
	item=strtok(NULL, " ");	//item now contains first number
	if(check_number(item))
	{
		*num1=atoi(item);
		item=strtok(NULL, "\n"); //item now contains second number
		if(check_number(item))
		{
			*num2=atoi(item);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

}

int add(int num1, int num2)
{
	return num1+num2;
}

double division(int num1, int num2)
{
	return (double)num1/num2;
}

int sub(int num1, int num2)
{
	return num1-num2;
}

int mult(int num1, int num2)
{
	return num1*num2;
}

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);
		setbuf(stderr, NULL);
	int port;
	if(argc > 1)
	{
		port = atoi(argv[1]);
	}
	else
		port = PROTOPORT;
	if (port < 0)
	{
		printf("Bad port number %s \n", argv[1]);
		return 0;
	}

	#if defined WIN32
		WSADATA wsa_data;
		int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
		if(result != 0)
		{
			errorhandler("Error at WSAStartup() \n");
			return 0;
		}
	#endif

	//welcome socket
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(my_socket < 0)
	{
		errorhandler("Socket creation failed. \n");
		clearWinsock();
		return -1;
	}

	//connection settings and listen
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(port);
	if(bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0)
	{
		errorhandler("bind() failed. \n");
		system("pause");
		closesocket(my_socket);
		clearWinsock();
		return -1;
	}

	if(listen(my_socket, QLEN) < 0)
	{
		errorhandler("listen() failed. \n");
		system("pause");
		closesocket(my_socket);
		clearWinsock();
		return -1;
	}

	//new connection
	struct sockaddr_in cad;
	int client_socket;
	int client_len;
	puts("Waiting for a client to connect... \n");

	while(1)
	{
		client_len = sizeof(cad);
		if((client_socket = accept(my_socket, (struct sockaddr*) &cad, &client_len)) < 0)
		{
			errorhandler("accept() failed. \n");
			system("pause");
			closesocket(my_socket);
			clearWinsock();
			return 0;
		}
		else
		{
			printf("Connection established with %s:%d \n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

			//send "Connected!"
			char* connected="Connected!";
			int lenght = strlen(connected);
			if (send(client_socket, connected, lenght, 0) != lenght)
			{
				errorhandler("client-send() sent a different number of bytes than expected");
				system("pause");
				closesocket(my_socket);
				clearWinsock();
			}
			else
			{
					int bytes, num1, num2, result;
					double result1;	//result1 contains division result: it might be a double type number
					char operation[BUFFER_SIZE], result_string[BUFFER_SIZE];	//result_string contains result converted into string
					memset(operation, '\0', BUFFER_SIZE);
					if ((bytes = recv(client_socket, operation, BUFFER_SIZE-1, 0)) <= 0)
					{
						errorhandler("recv() failed or connection closed prematurely");
						system("pause");
						closesocket(my_socket);
						clearWinsock();
						return -1;
					}
					else
					{
						while(strcmp(operation, "=")!=0)
						{
							if(strlen(operation)!=0)
							{
								char* item=strtok(operation, " ");	//item now contains operator
								if(item!=NULL) //item is null if the input contains only spaces
								{
									char op=item[0];
									switch(op)
									{
										case '+':
											if(token(item, &num1, &num2))
											{
												result=add(num1, num2);
												sprintf(result_string, "%d", result);	//sprintf converts result from int to string
												send_result(result_string, client_socket, my_socket);
											}
											else	//if token returns 0
												send_error(client_socket, my_socket);
											break;

										case '-':
											if(token(item, &num1, &num2))
											{
												result=sub(num1, num2);
												sprintf(result_string, "%d", result);
												send_result(result_string, client_socket, my_socket);
											}
											else
												send_error(client_socket, my_socket);
											break;

										case 'x':
											if(token(item, &num1, &num2))
											{
												result=mult(num1, num2);
												sprintf(result_string, "%d", result);
												send_result(result_string, client_socket, my_socket);
											}
											else
												send_error(client_socket, my_socket);
											break;

										case '/':
											if(token(item, &num1, &num2))
											{
												result1=division(num1, num2);
												sprintf(result_string, "%f", result1); //sprintf converts result from double to string
												send_result(result_string, client_socket, my_socket);
											}
											else
												send_error(client_socket, my_socket);
											break;

										default:	//unknown operator
											send_error(client_socket, my_socket);
									}	//end switch
								}
							else	//only spaces
								send_error(client_socket, my_socket);
							}
							memset(operation, '\0', BUFFER_SIZE);
							if ((bytes = recv(client_socket, operation, BUFFER_SIZE-1, 0)) <= 0)	//new operation
							{
								errorhandler("recv() failed or connection closed prematurely");
								system("pause");
								closesocket(my_socket);
								clearWinsock();
								return -1;
							}
						} //end while
					}
				}
			}
		} //end while(1)
} //end main
