NAME = test_config_parser

CXX = c++
CXXFLAGS = #-Wall -Wextra -Werror -std=c++11 -I./include -g
HEADERDIR = include
SRCDIR = src
OBJDIR = obj
CONFIG_SRC = $(SRCDIR)/config
CONFIG_FILES_DIR = config_files

SOURCES = $(CONFIG_SRC)/Configuration.cpp $(CONFIG_SRC)/Lexer.cpp $(CONFIG_SRC)/ParseError.cpp $(CONFIG_SRC)/Parser.cpp $(CONFIG_SRC)/PrintUtilities.cpp $(CONFIG_SRC)/Utilities.cpp $(CONFIG_SRC)/DirectiveSpecs.cpp $(CONFIG_SRC)/Validator.cpp
TESTER = tester/Tester.cpp
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