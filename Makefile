NAME = test_lexer

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -I./include
SRCDIR = parser
OBJDIR = obj

SOURCES = $(SRCDIR)/Lexer.cpp $(SRCDIR)/Parser.cpp $(SRCDIR)/tester.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)

test: $(NAME)
	./$(NAME) $(SRCDIR)/example.conf
	echo "----------------------------------------"
	./$(NAME) $(SRCDIR)/example2.conf

clean:
	rm -rf $(OBJDIR) $(NAME)

re: clean all

.PHONY: all test clean