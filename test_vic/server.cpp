/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmoreau <vmoreau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/06 14:08:02 by vmoreau           #+#    #+#             */
/*   Updated: 2021/12/06 17:47:37 by vmoreau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/color.hpp"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

void clear_buf(char buff[1024])
{
	int i = 0;

	while (i < 1024)
	{
		buff[i] = '\0';
		i++;
	}
}

int main()
{
	int server_fd, new_sock;
	struct sockaddr_in address;
	int opt = 1;
	int addr_size = sizeof(address);
	char buffer[10024] = {0};
	std::string msg, recep;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << RED << "Error=> " << NC << "Socket creation error\n";
		exit(EXIT_FAILURE);
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << RED << "Error=> " << NC << "Setsockopt error\n";
		exit(EXIT_FAILURE);
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		std::cerr << RED << "Error=> " << NC << "Setsockopt error\n";
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8080);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << RED << "Error=> " << NC << "Bind failed\n";
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		std::cerr << RED << "Error=> " << NC << "listen\n";
		exit(EXIT_FAILURE);
	}
	std::cout << "Server setup on port 8080 and wating for client connect:\n\n";
	if ((new_sock = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addr_size)) < 0)
	{
		std::cerr << RED << "Error=> " << NC << "accepte\n";
		exit(EXIT_FAILURE);
	}
	// msg = "GET ./HTTP/index.html HTTP/1.1 200 ok\n";
	msg = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-type: text/html\r\n\r\n<html>\r\n<head>\r\n<title>Hello, world!</title>\r\n</head>\r\n<body>\r\n<h1>TOTO!</h1>\r\n</body>\r\n</html>\r\n\r\n";
	// msg = "GET HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-type: text/html\r\n\r\n<html>\r\n<head>\r\n<title>Hello, world!</title>\r\n</head>\r\n<body>\r\n<h1>Hello, world!</h1>\r\n</body>\r\n</html>\r\n\r\n";
	while (1)
	{
		clear_buf(buffer);
		recv(new_sock, buffer, 10024, 0);
		recep = buffer;
		std::cout << YELLOW << "Client:		" << NC << recep << '\n';
		if (send(new_sock, msg.c_str(), msg.size() , 0) < 0)
		{
			std::cerr << RED << "Error=> " << NC << "Message not send\n";
			exit(EXIT_FAILURE);
		}
	}
	return (0);
}