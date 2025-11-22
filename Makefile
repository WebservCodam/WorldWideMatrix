NAME = test_config_parser

CXX = c++
CXXFLAGS = #-Wall -Wextra -Werror -std=c++11 -I./include -g
HEADERDIR = include
SRCDIR = src
OBJDIR = obj
CONFIG_FILES_DIR = config_files

SOURCES = $(SRCDIR)/config/Configuration.cpp $(SRCDIR)/lexer/Lexer.cpp $(SRCDIR)/parser/ParseError.cpp $(SRCDIR)/parser/Parser.cpp $(SRCDIR)/utils/PrintUtilities.cpp $(SRCDIR)/utils/Utilities.cpp $(SRCDIR)/validator/DirectiveSpecs.cpp $(SRCDIR)/validator/Validator.cpp
TESTER = $(SRCDIR)/tester/Tester.cpp
SOURCES += $(TESTER)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)

clean:
	rm -rf $(OBJDIR)

fclean:	clean
	rm -f $(NAME)

re: clean all

.PHONY: all test clean tester