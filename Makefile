LEX := flex
YACC := bison
CC := g++
CFLAGS := -g -Wall -O2
TARGET := main.exe
SRCS := $(wildcard *.cpp Utils/*.cpp Record/*.cpp)

all: $(TARGET)

$(TARGET): $(SRCS) Parser/yacc.tab.c Parser/lex.yy.c
	$(CXX) $(CFLAGS) -o $@ $^

Parser/yacc.tab.c: Parser/yacc.y
	$(YACC) -d Parser/yacc.y -b Parser/yacc

Parser/lex.yy.c: Parser/lex.l
	$(LEX) -o Parser/lex.yy.c Parser/lex.l

clean:
	del Parser\yacc.tab.c, Parser\yacc.tab.h, Parser\lex.yy.c, $(TARGET)
