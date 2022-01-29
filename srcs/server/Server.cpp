/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mabriand <mabriand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/13 19:18:35 by vmoreau           #+#    #+#             */
/*   Updated: 2022/01/29 22:27:40 by mabriand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Server.hpp"
#include "../cgi/CgiProcess.hpp"
#include "../response/Response.hpp"

std::string ipToString(unsigned int ip)
{
	unsigned char		v[4];
	std::ostringstream	os;

	for (int i = 0; i < 4; ++i)
	{
		v[i] = (ip >> (i * 8)) & 0xFF;
	}

	for (int i = 0; i < 3; ++i)
	{
		os << static_cast<int>(v[i]);
		os << '.';
	}
	os << static_cast<int>(v[3]);

	return (os.str());
}

int Server::server_is_alive = 1;

Server::Server(/* args */)
{
}

Server::~Server()
{
	for (std::map< int, Request *>::iterator it = this->_client_sock.begin(); it != this->_client_sock.end(); it++)
	{
		close(it->first);
		delete(it->second);
	}
	this->Server_closeAllSocket();
	this->Server_Zero_all_set();
}

void Server::Server_Zero_all_set()
{
	FD_ZERO(&this->_currentfds);
	FD_ZERO(&this->_readfds);
	FD_ZERO(&this->_writefds);
}

void Server::Server_closeSocket(int socket)
{
	if (socket >= 0)
		close(socket);
}

void Server::Server_closeAllSocket()
{
	for (std::map< int, sockaddr_in >::iterator it = this->_socket.begin(); it != this->_socket.end(); it++)
		this->Server_closeSocket(it->first);
	std::cout << YELLOW << "All opened Sockets have been closed.\n" << NC;
}

void Server::Server_setSocket()
{
	for (size_t i = 0; i < this->_servers.size(); i++)
	{
		int	mysock;
		int opt = 1;

		memset((char *)&mysock, 0, sizeof(mysock));
		if ((mysock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			this->Server_closeAllSocket();
			throw ServerError("Socket creation error");
		}
		if (fcntl(mysock, F_SETFL, O_NONBLOCK) < 0)
			std::cout << "Fcntl failed. errno: " << errno << std::endl;

		if (setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			this->Server_closeAllSocket();
			throw ServerError("Setsockopt \"SO_REUSEADDR\" error");
		}
		if (setsockopt(mysock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		{
			this->Server_closeAllSocket();
			throw ServerError("Setsockopt \"SO_REUSEPORT\" error");
		}

		struct sockaddr_in		myaddr;

		int addr_size = sizeof(myaddr);
		memset((char *)&myaddr, 0, addr_size);
		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(this->_servers[i].get_port());
		myaddr.sin_addr.s_addr = inet_addr(this->_servers[i].get_host().c_str());

		std::cerr << "IP: " << ipToString(myaddr.sin_addr.s_addr) << std::endl;
		std::cerr << "PORT: " << this->_servers[i].get_port() << std::endl;

		if (bind(mysock, (struct sockaddr *)&myaddr, addr_size) < 0)
		{
			this->Server_closeAllSocket();
			throw ServerError(std::strerror(errno));
		}

		int	backlog = 1;

		if (listen(mysock, backlog) < 0)
		{
			this->Server_closeAllSocket();
			throw ServerError("Couldn't listen to socket.");
		}
		this->_socket.insert(std::make_pair(mysock, myaddr));
	}
}

void Server::Server_setFd()
{
	FD_ZERO(&this->_currentfds);
	for (std::map< int, sockaddr_in >::iterator it = this->_socket.begin(); it != this->_socket.end(); it++)
		FD_SET(it->first, &this->_currentfds);
	this->_nfds = this->_socket.rbegin()->first;
}


void Server::Server_init(confpars *html, std::vector< serv_block > serv)
{
	this->_html = html;
	this->_servers = serv;
}

void Server::Server_select()
{
	this->_readfds = this->_currentfds;
	this->_writefds = this->_currentfds;

	// std::cout << "P1\n";
	// this->print_fds(YELLOW);

	int ret_select = select(this->_nfds + 1, &this->_readfds, &this->_writefds, NULL, NULL);
	// std::cout << "P2\n";

	if (ret_select < 0 && (errno != EINTR))
		throw ServerError(std::strerror(errno));

	this->_rdy_fd = ret_select;

	// this->print_fds(PURPLE);
}

void Server::Server_loopServ()
{
	for (int i = 0; i <= this->_nfds; i++)
	{
		if (FD_ISSET(i, &this->_readfds))
		{
			this->_rdy_fd--;
			if (this->_socket.find(i) != this->_socket.end())
			{
				std::map<int, sockaddr_in>::iterator it = this->_socket.find(i);
				int client_socket = accept(it->first, NULL, NULL);
				if (client_socket < 0 && errno != EAGAIN)
				{
					std::cerr << client_socket << "  " << EAGAIN << '\n';
					std::cerr << RED << "Server error: " << NC << "Couldn't accept connection.\n";
					std::cerr << std::strerror(errno);
					exit(EXIT_FAILURE);
				}
				else if (client_socket > 0)
				{
					std::cout << YELLOW << "New incoming connection (fd " << client_socket << ")" << " for -> " << i << NC << std::endl;
					FD_SET(client_socket, &this->_currentfds);

					Request* req = new Request(client_socket, &this->_servers[i - 3]);

					this->_client_sock.insert(std::make_pair(client_socket, req));

					if (client_socket > this->_nfds)
						this->_nfds = client_socket;
				}
			}
		}
	}
}

void Server::Server_loopClient()
{
	for (std::map<int, Request*>::iterator it = this->_client_sock.begin(); it != this->_client_sock.end() && this->_rdy_fd > 0; )
	{
		int ret = 0;
		if (FD_ISSET(it->first, &this->_readfds))
		{
			ret = it->second->parse();
			if (ret >= 0 && it->second->is_request_ready() == true)
			{
				CgiProcess newProcess(it->second, this);
				if (newProcess.isCgiNeeded() == true)
					newProcess.exeCgiProgram();
				// Response	resp(it->second->returnProtocolVersion(), it->second->returnStatusCode(), it->second->returnUrl(), it->second->getBlock());
				Response	resp(it->second);
				if (newProcess.isCgiNeeded() == true)
					resp.set_cgiBody(newProcess.get_cgiBody());
				this->_response = resp.getVecResponse();
			}
		}
		if (FD_ISSET(it->first, &this->_writefds) && this->_response.size())
		{
			ret = send(it->first, &this->_response[0], this->_response.size() , MSG_DONTWAIT);

			if (ret == -1)
			{
				std::cout << PURPLE << "AIE AIE AIE !!!\n" << NC;
			}

			// std::cout << "AFTER SEND: " << ret << NC << std::endl;

			this->_response.erase(this->_response.begin(), this->_response.begin() + ret);

			if (this->_response.size() == 0)
			{
				std::cout << RED << "connection closed(fd " << it->first << ")" << NC << std::endl;
				FD_CLR(it->first, &this->_currentfds);
				close(it->first);
				delete(it->second);
				std::map<int, Request*>::iterator tmp = it;
				it++;
				this->_client_sock.erase(tmp);
			}
			continue;
		}
		it++;
	}
}

void Server::Server_launch()
{
		this->_rdy_fd = 0;
		Server_setSocket();
		Server_setFd();

		int i = 0; // debug
		while (Server::server_is_alive && i <= 50)
		{
			this->Server_select();
			this->Server_loopServ();
			this->Server_loopClient();
			// i++; // debug
		}
}


// --------------- DEBUG --------------- //
void Server::print_fds(const char *color)
{
	std::cout << color << "\nFD_SET in current:\n";
	for (int i = 0; i <= this->_nfds; i++)
	{
		if (FD_ISSET(i, &this->_currentfds))
			std::cout << i << ' ';
	}
	std::cout << '\n';
		std::cout << "FD_SET in read:\n";
	for (int i = 0; i <= this->_nfds; i++)
	{
		if (FD_ISSET(i, &this->_readfds))
			std::cout << i << ' ';
	}
	std::cout << '\n';
		std::cout << "FD_SET in write:\n";
	for (int i = 0; i <= this->_nfds; i++)
	{
		if (FD_ISSET(i, &this->_writefds))
			std::cout << i << ' ';
	}
	std::cout << "\n\n" << NC;
}
