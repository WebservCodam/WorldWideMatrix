CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -I./include
SRCDIR = parser
OBJDIR = obj

SOURCES = $(SRCDIR)/Lexer.cpp $(SRCDIR)/Parser.cpp $(SRCDIR)/test_parser.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = parser_test

all: $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

test: $(TARGET)
	./$(TARGET) $(SRCDIR)/example.conf
	echo "----------------------------------------"
	./$(TARGET) $(SRCDIR)/example2.conf

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all test clean