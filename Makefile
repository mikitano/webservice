# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/18 21:08:56 by mkitano           #+#    #+#              #
#    Updated: 2026/11/21 10:50:47 by mkitano          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

# Compiler
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CPPFLAGS = -Iinclude
DEPFLAGS = -MMD -MP

# Directories
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Source files
SRC_MAIN = $(addsuffix .cpp, main)
SRC_LEXER = $(addsuffix .cpp, Lexer Token)
SRC_MODEL = $(addsuffix .cpp, Config LocationConfig ServerConfig)
SRC_PARSER = $(addsuffix .cpp, Parser)
#SRC_PARSER = $(addsuffix .cpp, Parser LocationParser ServerParse)

SRC = \
	$(addprefix $(SRC_DIR)/, $(SRC_MAIN)) \
	$(addprefix $(SRC_DIR)/config/lexer/, $(SRC_LEXER)) \
	$(addprefix $(SRC_DIR)/config/model/, $(SRC_MODEL)) \
	$(addprefix $(SRC_DIR)/config/parser/, $(SRC_PARSER)) \


# Object files
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# Dependency files
DEP = $(OBJ:.o=.d)

# Main target (temporary: compile production modules)
all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "\033[0;32mWebserv compiled successfully!\033[0m"

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "\033[0;33mCompiling: $<\033[0m"
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

# Include header dependencies
-include $(DEP)

# Remove object files
clean:
	@rm -rf $(OBJ_DIR)
	@echo "\033[0;31mObject files removed!\033[0m"

# Remove objects and executable
fclean: clean
	@rm -f $(NAME)
	@echo "\033[0;31mExecutable removed!\033[0m"

# Rebuild
re: fclean all

.PHONY: all clean fclean re