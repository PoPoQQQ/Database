LEX := flex
YACC := bison
CC := g++
CFLAGS := -g -std=c++11 -O2
TARGET := main.exe
SRCS := $(wildcard *.cpp Utils/*.cpp Index/*.cpp Record/*.cpp FieldConstraint/*.cpp Parser/*.cpp Pages/*.cpp)

all: $(TARGET)

$(TARGET): $(SRCS) Parser/yacc.tab.c Parser/lex.yy.c
	$(CXX) $(CFLAGS) -o $@ $^

Parser/yacc.tab.c: Parser/yacc.y
	$(YACC) -d Parser/yacc.y -b Parser/yacc

Parser/lex.yy.c: Parser/lex.l
	$(LEX) -o Parser/lex.yy.c Parser/lex.l

clean:
	del Parser\yacc.tab.c, Parser\yacc.tab.h, Parser\lex.yy.c, $(TARGET)

love:
	rd Database /s/q

# linux
# STATICLIB=test_lib.a
# SHARELIB=test_lib.so
# LIB:=lib
# MAIN:=main
# ###
# Target := target
# SRCDIR = BufManager FileIO Index Pages Parser Record Utils
# OBJDIR = obj
# SRCPATH:= $(filter-out main.cpp,$(foreach dir,$(SRCDIR),$(wildcard $(addprefix ${dir}/*,.cpp))))
# INCLUDEPATH := $(SRCDIR)
# OBJS:= $(addprefix $(OBJDIR)/, $(subst .cpp, .o, $(foreach dir,$(SRCDIR),$(wildcard *.cpp))))

# $(MAIN): main.cpp obj $(INCLUDEPATH) 
# 	@test -d $(Target) | mkdir -p $(Target)
# 	g++ $(OBJS) main.cpp -o $@ -I $(INCLUDEPATH) $(CFLAGS)
# 	@mv $(MAIN) $(Target)/
 
# $(OBJS) :$(SRCPATH)
# 	@test -d $(OBJDIR) | mkdir -p $(OBJDIR)
# 	g++ -c $< $(CFLAGS) -o $@
# 	@mv *.o $(OBJDIR)/
 
# $(STATICLIB): obj
# 	@test -d $(LIB) | mkdir -p $(LIB)
# 	ar -q $@ $(OBJS)
# 	@mv $@ $(LIB)/

# $(SHARELIB):$(SRCPATH)
# 	@test -d $(LIB) | mkdir -p $(LIB)
# 	g++ $< -shared -fPIC  -o $@
# 	@mv $(SHARELIB) $(LIB)/

.PHONY: rmps rmdb test sqltest

rmps:
	rm ./Parser/yacc.tab.c Parser/yacc.tab.h Parser/lex.yy.c $(TARGET)
rmdb:
	rm -rf Database

test: rmdb all
	./main.exe

sqltest: rmps rmdb all
	./main.exe test.sql

# clean_cache:
# 	rm $(OBJDIR) -rf
# 	rm $(LIB) -rf
# 	rm $(Target) -rf