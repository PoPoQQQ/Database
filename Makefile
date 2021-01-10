LEX := flex
YACC := bison
CC := g++
CFLAGS := -g -std=c++11
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
# # Target := target
# SRCDIR = BufManager FileIO Index Pages Parser Record Utils
# OBJDIR = obj
# SRCPATH:= $(filter-out main.cpp,$(foreach dir,$(SRCDIR),$(wildcard $(addprefix ${dir}*,.cpp))))
# INCLUDEPATH := $(SRCDIR)
# OBJS:= $(wildcard $(OBJDIR)/*.o)

# $(MAIN): main.cpp obj YACC LEX $(INCLUDEPATH) 
# 	# @test -d $(Target) | mkdir -p $(Target)
# 	g++ $(OBJS) Parser/yacc.tab.c Parser/lex.yy.c main.cpp -o $@ $(foreach dir,$(INCLUDEPATH), $(addprefix -I , ${dir})) $(CFLAGS)
# 	# @mv $(MAIN) $(Target)/
 
# obj :$(SRCPATH)
# 	@test -d $(OBJDIR) | mkdir -p $(OBJDIR)
# 	g++ -c $(SRCPATH) $(CFLAGS)
# 	@mv *.o $(OBJDIR)/

# YACC: Parser/yacc.y
# 	$(YACC) -d Parser/yacc.y -b Parser/yacc

# LEX: Parser/lex.l
# 	$(LEX) -o Parser/lex.yy.c Parser/lex.l

.PHONY: rmps rmdb test sqltest

rmps:
	rm ./Parser/yacc.tab.c Parser/yacc.tab.h Parser/lex.yy.c
rmdb:
	rm -rf Database

test: rmdb all
	./main.exe

load: all
	./main.exe Tools/build.sql
	./main.exe Tools/part.tbl.sql
	./main.exe Tools/region.tbl.sql
	./main.exe Tools/nation.tbl.sql
	./main.exe Tools/supplier.tbl.sql
	./main.exe Tools/customer.tbl.sql
	./main.exe Tools/partsupp.tbl.sql
	./main.exe Tools/orders.tbl.sql
	./main.exe Tools/lineitem.tbl.sql

sqltest: rmps rmdb all
	./main.exe test.sql

clean_cache:
	rm $(OBJDIR) -rf
	rm main