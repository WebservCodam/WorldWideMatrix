NAME		=		server
CONFIG_SRC = 		configparser/src/config
SRC			=		Client.cpp\
					init.cpp\
					main.cpp\
					Server.cpp\
					utils.cpp\
					parser/Body.cpp\
					parser/Headers.cpp\
					parser/HttpParser.cpp\
					parser/RequestLine.cpp\
					parser/utils2.cpp\
					$(CONFIG_SRC)/ConfigError.cpp \
					$(CONFIG_SRC)/Configuration.cpp \
					$(CONFIG_SRC)/DirectiveSpecs.cpp \
					$(CONFIG_SRC)/Lexer.cpp \
					$(CONFIG_SRC)/Parser.cpp \
					$(CONFIG_SRC)/ServerConfig.cpp \
					$(CONFIG_SRC)/Validator.cpp
OBJ			=		$(SRC:.cpp=.o)
CC			=		c++
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
			./server && siege -t $(V1)s -c $(V2) http://localhost:8080

o:
			./server > output.txt 2>&1

.PHONY:		all clean fclean re test teststart o