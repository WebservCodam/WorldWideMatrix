NAME		=			webserver
SRC_DIR 	=			src
CONFIG_SRC_DIR = 		$(SRC_DIR)/configparser
VALIDATION_SRC_DIR =	$(CONFIG_SRC_DIR)/validation_functions

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
					$(CONFIG_SRC_DIR)/Utils.cpp \
					$(VALIDATION_SRC_DIR)/AutoindexDirective.cpp \
					$(VALIDATION_SRC_DIR)/CGIDirectives.cpp \
					$(VALIDATION_SRC_DIR)/ClientMaxBodySizeDirective.cpp \
					$(VALIDATION_SRC_DIR)/ErrorPageDirective.cpp \
					$(VALIDATION_SRC_DIR)/IndexDirective.cpp \
					$(VALIDATION_SRC_DIR)/KeepaliveTimeout.cpp \
					$(VALIDATION_SRC_DIR)/ListenDirective.cpp \
					$(VALIDATION_SRC_DIR)/MethodsDirective.cpp \
					$(VALIDATION_SRC_DIR)/ReturnDirective.cpp \
					$(VALIDATION_SRC_DIR)/RootDirective.cpp \
					$(VALIDATION_SRC_DIR)/Utilities.cpp
OBJ			=		$(SRC:.cpp=.o)
CC			=		c++
FLAGS		= -g
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
