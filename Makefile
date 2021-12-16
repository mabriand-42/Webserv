# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mabriand <mabriand@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/11/29 16:52:41 by vmoreau           #+#    #+#              #
#    Updated: 2021/12/16 15:28:19 by mabriand         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = Serv

####################################COLOR######################################
#----------------reset----------------#
NC = \033[0m

#-----------Regular Colors------------#
BLACK = \033[0;30m
RED = \033[0;31m
GREEN = \033[32m
YELLOW = \033[33;33m
BLUE = \033[0;34m
PURPLE = \033[35m
CYAN = \033[1;36m
WHITE = \033[0;37m

################################COMMON  SOURCES################################

SRCS += srcs/main.cpp

# Conf
SRCS += srcs/conf/confpars.cpp srcs/conf/serv_block.cpp srcs/conf/loc_block.cpp
# Request
SRCS += srcs/request/Request.cpp
# Response
SRCS += srcs/response/Response.cpp
# Server
SRCS += srcs/server/Server.cpp
####################################BASIC######################################

CFLAGS = -Wall -Werror -Wextra -std=c++98

# CFLAGS += -g3 -fsanitize=address

CC = clang++

INC = incs/

HEADER = $(INC)

OBJ = $(SRCS:.cpp=.o)

#####################################RULE######################################

all : $(NAME)

$(NAME) : echoCW $(OBJ) echoOK echoCS
	$(CC) $(CFLAGS) -o $@ $(OBJ)


%.o: %.cpp $(HEADER)
	$(CC) -c $(CFLAGS) -I $(INC) $< -o $@
	printf "$(GREEN)██"

clean :	echoCLEAN
	$(RM) $(OBJ)

fclean : clean echoFCLEAN
	$(RM) $(NAME)

re : fclean all

####################################ECHO######################################

echoCW:
	echo "$(YELLOW)===> Compiling Webserv$(NC)"
echoOK:
	echo "$(GREEN)OK$(NC)"
echoCS :
	echo "$(GREEN)===> Compilation Success$(NC)"
echoCLEAN :
	echo "$(PURPLE)===> Cleanning OBJ$(NC)"
echoFCLEAN :
	echo "$(PURPLE)===> Cleanning Execs$(NC)"


.PHONY : all clean fclean re

.SILENT :
