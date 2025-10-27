NAME = tester

CXX = c++
CXXFLAGS = #-Wall -Wextra -Werror -std=c++11 -I./include -g
HEADERDIR = include
SRCDIR = parser
OBJDIR = obj

HEADERS = $(HEADERDIR)/Configuration.hpp $(HEADERDIR)/DirectiveSpecs.hpp $(HEADERDIR)/Lexer.hpp $(HEADERDIR)/ParseError.hpp $(HEADERDIR)/Parser.hpp $(HEADERDIR)/Validator.hpp
SOURCES = $(SRCDIR)/DirectiveSpecs.cpp $(SRCDIR)/Lexer.cpp $(SRCDIR)/ParseError.cpp $(SRCDIR)/Parser.cpp $(SRCDIR)/PrintUtilities.cpp $(SRCDIR)/tester.cpp $(SRCDIR)/Validator.cpp 
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
	rm -rf $(OBJDIR)

fclean:	clean
	$(NAME)

re: clean all

.PHONY: all test clean