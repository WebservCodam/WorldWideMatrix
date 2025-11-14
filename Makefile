NAME		=		server
SRC			=		Client.cpp\
					init.cpp\
					main.cpp\
					Server.cpp\
					utils.cpp
OBJ			=		$(SRC:.cpp=.o)
CC			=		c++
FLAGS		=		-Wall -Wextra -Werror -g
# FLAGS		+=		-fsanitize=thread
# FLAGS		+=		-fsanitize=address
ARGS		=	
V1			= 		10
V2			=		100	

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

.PHONY:		all clean fclean re test teststart