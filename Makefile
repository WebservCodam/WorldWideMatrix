NAME		=			webserv
SRC_DIR 	=			src
CONFIG_SRC_DIR = 		$(SRC_DIR)/configparser
DIRECTIVES_SRC_DIR =	$(CONFIG_SRC_DIR)/directives_functions
HTTP_SRC_DIR =			$(SRC_DIR)/httpparser

SRC			=		$(SRC_DIR)/Cgi.cpp \
					$(SRC_DIR)/Client.cpp \
					$(SRC_DIR)/init.cpp \
					$(SRC_DIR)/main.cpp \
					$(SRC_DIR)/Webserv.cpp \
					$(SRC_DIR)/Server.cpp \
					$(SRC_DIR)/utils.cpp \
					$(HTTP_SRC_DIR)/Body.cpp \
					$(HTTP_SRC_DIR)/Headers.cpp \
					$(HTTP_SRC_DIR)/HttpException.cpp \
					$(HTTP_SRC_DIR)/HttpParser.cpp \
					$(HTTP_SRC_DIR)/RequestLine.cpp \
					$(HTTP_SRC_DIR)/utils2.cpp \
					$(CONFIG_SRC_DIR)/Directive.cpp \
					$(CONFIG_SRC_DIR)/ConfigError.cpp \
					$(CONFIG_SRC_DIR)/ConfigFile.cpp \
					$(CONFIG_SRC_DIR)/Validation.cpp \
					$(CONFIG_SRC_DIR)/Lexer.cpp \
					$(CONFIG_SRC_DIR)/Parser.cpp \
					$(CONFIG_SRC_DIR)/ServerConfig.cpp \
					$(CONFIG_SRC_DIR)/Utilities.cpp \
					$(DIRECTIVES_SRC_DIR)/AutoindexDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/CGIDirectives.cpp \
					$(DIRECTIVES_SRC_DIR)/ClientMaxBodySizeDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/ErrorPageDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/IndexDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/KeepaliveTimeout.cpp \
					$(DIRECTIVES_SRC_DIR)/ListenDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/LocationDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/MethodsDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/ReturnDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/RootDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/ServerDirective.cpp \
					$(DIRECTIVES_SRC_DIR)/UploadPathDirective.cpp
OBJ			=		$(SRC:.cpp=.o)
CC			=		c++
FLAGS		=		-Wall -Wextra -Werror -g -std=c++17

MAKEFLAGS= -j

$(NAME):	$(OBJ)
				$(CC) $(FLAGS) -o $(NAME) $(OBJ)
				@mkdir -p www/uploads
				@rm -f $(OBJ)

%.o: %.cpp	
	$(CC) $(FLAGS) -c $< -o $@

all:		$(NAME)

clean:
				@rm -f $(OBJ)

fclean:		clean
				@rm -f $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
