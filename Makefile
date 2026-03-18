NAME		=			webserver
SRC_DIR 	=			src
CONFIG_SRC_DIR = 		$(SRC_DIR)/configparser
DIRECTIVES_SRC_DIR =	$(CONFIG_SRC_DIR)/directives_functions

SRC			=		$(SRC_DIR)/Client.cpp\
					$(SRC_DIR)/init.cpp\
					$(SRC_DIR)/main.cpp\
					$(SRC_DIR)/Server.cpp\
					$(SRC_DIR)/utils.cpp\
					parser/Body.cpp\
					parser/Headers.cpp\
					parser/HttpParser.cpp\
					parser/RequestLine.cpp\
					parser/utils2.cpp\
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
					$(DIRECTIVES_SRC_DIR)/ServerDirective.cpp 
OBJ			=		$(SRC:.cpp=.o)
CC			=		c++
FLAGS		=		-g
# FLAGS		=		-Wall -Wextra -Werror -g
# FLAGS		+=		-fsanitize=thread
# FLAGS		+=		-fsanitize=address
ARGS		=	
V1			= 		10
V2			=		100

MAKEFLAGS= -j

$(NAME):	$(OBJ)
				$(CC) $(FLAGS) -o $(NAME) $(OBJ)
				@rm -f $(OBJ)

%.o: %.cpp	
	$(CC) $(FLAGS) -c $< -o $@

all:		$(NAME)

clean:
				@rm -f $(OBJ)

fclean:		clean
				@rm -f $(NAME)

re:			fclean all

run:		all
#				@./$(NAME) $(ARGS)
#				@bash ./test.sh
#				@valgrind --leak-check=full ./$(NAME) $(ARGS)

test:
			siege -t $(V1)s -c $(V2) http://localhost:8080

teststart:
			./webserver config_files/valid/example.conf && siege -t $(V1)s -c $(V2) http://localhost:8080

o:
			./webserver config_files/valid/example.conf > output.txt 2>&1

.PHONY:		all clean fclean re test teststart o
