NAME = server

CXX = c++

SRC = main.cpp

FLAGS = -std=c++11

OBJ_DIR = obj

OBJ_FILES := $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@$(CXX) $(FLAGS) $^ -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(FLAGS) -MMD -MP -c $< -o $@

-include $(OBJ_FILES:.o=.d)

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

# Colors
GREEN=\033[0;32m
GREEN_LIGHT=\033[1;32m
YELLOW=\033[0;33m
GOLD=\033[38;5;220m
RESET=\033[0m

