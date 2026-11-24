# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/14 10:57:02 by namatias          #+#    #+#              #
#    Updated: 2026/11/23 21:25:35 by namatias         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

################################################################################
#                       FLAGS AND VARIABLES TO COMPILE                         #
################################################################################

NAME	= webserv

CPP			= c++
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98 -MMD -MP

OBJ_DIR = ./obj/
INC_DIR = ./include/
SRC_DIR = ./src/

INCLUDES = -I$(INC_DIR)

SRC_FILES = 	main.cpp \
				network/SocketServer.cpp \
				utils/Logger.cpp \
				utils/Utils.cpp \

SRCS 		= $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJS		= $(addprefix $(OBJ_DIR), $(SRC_FILES:.cpp=.o))

ARGS		= $(PORT1) $(PORT2)
PORT1		= 4242
PORT2		= 4243

################################################################################
#                       TESTES DAS CLASSES (src/tests/)                        #
################################################################################
# make test_loop    -> testa só o LoopingEvent   (./test_loop)
# make test_client  -> testa só o Client         (./test_client)
# make test_server  -> roda o Server com config mock (./test_server 4242 4243)

TEST_LOOP		= test_loop
TEST_CLIENT		= test_client
TEST_SERVER		= test_server

OBJS_TEST_LOOP	= $(OBJ_DIR)tests/test_loopingevent.o \
						$(OBJ_DIR)network/LoopingEvent.o \
						$(OBJ_DIR)utils/Logger.o

OBJS_TEST_CLIENT	= $(OBJ_DIR)tests/test_client.o \
							$(OBJ_DIR)network/Client.o \
							$(OBJ_DIR)network/ClientInfo.o \
							$(OBJ_DIR)network/SocketServer.o \
							$(OBJ_DIR)utils/Utils.o \
							$(OBJ_DIR)utils/Logger.o

OBJS_TEST_SERVER	= $(OBJ_DIR)tests/test_server.o \
							$(OBJ_DIR)network/Server.o \
							$(OBJ_DIR)network/LoopingEvent.o \
							$(OBJ_DIR)network/Client.o \
							$(OBJ_DIR)network/ClientInfo.o \
							$(OBJ_DIR)network/SocketServer.o \
							$(OBJ_DIR)utils/Logger.o \
							$(OBJ_DIR)utils/Utils.o \
							$(OBJ_DIR)config/model/Config.o \
							$(OBJ_DIR)config/model/ServerConfig.o \
							$(OBJ_DIR)config/model/LocationConfig.o \

DEPS		= $(OBJS:.o=.d) $(OBJS_TEST_LOOP:.o=.d) $(OBJS_TEST_CLIENT:.o=.d) $(OBJS_TEST_SERVER:.o=.d)

################################################################################
#                                   COLORS                                     #
################################################################################

GREEN = \033[0;32m
RESET = \033[0m

################################################################################
#                                 Functions                                    #
################################################################################

all: $(NAME)

$(NAME): $(OBJS)
	@$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)
	@echo "[$(GREEN)OK$(RESET)] $(NAME) COMPILED!"

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	@$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@
	@echo "[$(GREEN)OK$(RESET)] OBJS COMPILED $<"

$(TEST_LOOP): $(OBJS_TEST_LOOP)
	@$(CPP) $(CPPFLAGS) $(OBJS_TEST_LOOP) -o $(TEST_LOOP)
	@echo "[$(GREEN)OK$(RESET)] $(TEST_LOOP) COMPILED! Rode: ./$(TEST_LOOP)"

$(TEST_CLIENT): $(OBJS_TEST_CLIENT)
	@$(CPP) $(CPPFLAGS) $(OBJS_TEST_CLIENT) -o $(TEST_CLIENT)
	@echo "[$(GREEN)OK$(RESET)] $(TEST_CLIENT) COMPILED! Rode: ./$(TEST_CLIENT)"

$(TEST_SERVER): $(OBJS_TEST_SERVER)
	@$(CPP) $(CPPFLAGS) $(OBJS_TEST_SERVER) -o $(TEST_SERVER)
	@echo "[$(GREEN)OK$(RESET)] $(TEST_SERVER) COMPILED! Rode: ./$(TEST_SERVER)"

clean:
	@rm -rf $(OBJ_DIR)
	@echo "[$(GREEN)OK$(RESET)] FILES CLEAR!"

fclean: clean
	@rm -f $(NAME) $(TEST_LOOP) $(TEST_CLIENT) $(TEST_SERVER)
	@echo "[$(GREEN)OK$(RESET)] ALL CLEAR!"

re: fclean all

val: re
	@valgrind 	--leak-check=full \
				--show-leak-kinds=all \
				--track-origins=yes \
				--track-fds=yes \
				--trace-children=yes \
				./${NAME} ${ARGS}

# Copiar e colar no terminal para testes de valgrind personal
# valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --trace-children=yes

# Recompila os .o quando um .hpp mudar (arquivos .d gerados por -MMD)
-include $(DEPS)

.PHONY: all clean fclean re val
