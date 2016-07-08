# Makefile
# Copyright (C) 2015 pexeer@gamil.com
# Wed Sep  9 19:45:01 CST 2015

STD=-std=c++11
WARNING=-Wall -Werror
DEBUG=-g -ggdb
#OPT=-O3

FINAL_CFLAGS=$(WARNING) $(OPT) $(DEBUG) $(CFLAGS)
FINAL_CXXFLAGS=$(STD) $(WARNING) $(OPT) $(DEBUG) $(CFLAGS) -I./include
FINAL_LDFLAGS=$(LDFLAGS)  $(DEBUG)
FINAL_LIBS=-lm -ldl -pthread

CCCOLOR="\033[34m"
LINKCOLOR="\033[34;1m"
SRCCOLOR="\033[33m"
BINCOLOR="\033[37;1m"
MAKECOLOR="\033[32;1m"
ENDCOLOR="\033[0m"

ifndef V
QUIET_C = @printf '    %b %b\n' $(CCCOLOR)CXX$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_LINK = @printf '%b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;
endif

P_CC=$(QUIET_C)$(CC) $(FINAL_CFLAGS)
P_CXX=$(QUIET_C)$(CXX) $(FINAL_CXXFLAGS)
P_LD=$(QUIET_LINK)$(CXX) $(FINAL_LDFLAGS)

p-base.a: src/utils.o src/endpoint.o src/socket.o
	ar cr $@ $^

src/%.o: src/%.cpp
	$(P_CXX) -o $@ -c $< -I./include

%.o: %.c
	$(P_CC) -c $<

%.o: %.cpp
	$(P_CXX) -c $<

%.exe: %.o p-base.a
	$(P_LD) -o $@ $^  $(FINAL_LIBS) p-base.a

all:
	@echo 'make target'

clean:
	rm -rf src/*.o *.o *.exe *.a
	@echo clean done

