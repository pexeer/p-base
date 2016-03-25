# Makefile
# Copyright (C) 2015 pexeer@gamil.com
# Wed Sep  9 19:45:01 CST 2015

STD=-std=c++11
WARN=-Wall -Werror
DEBUG=-g -ggdb
#OPT=-O3

FINAL_CFLAGS=$(WARN) $(OPT) $(DEBUG) $(CFLAGS)
FINAL_CXXFLAGS=$(STD) $(WARN) $(OPT) $(DEBUG) $(CFLAGS) -I./include
FINAL_LDFLAGS=$(LDFLAGS)  $(DEBUG)
FINAL_LIBS=-lm

FINAL_LIBS+= -pthread

CCCOLOR="\033[34m"
LINKCOLOR="\033[34;1m"
SRCCOLOR="\033[33m"
BINCOLOR="\033[37;1m"
MAKECOLOR="\033[32;1m"
ENDCOLOR="\033[0m"

QUIET_C = @printf '    %b %b\n' $(CCCOLOR)CXX$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_LINK = @printf '%b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;

P_CC=$(QUIET_C)$(CC) $(FINAL_CFLAGS)
P_CXX=$(QUIET_C)$(CXX) $(FINAL_CXXFLAGS)
P_LD=$(QUIET_LINK)$(CXX) $(FINAL_LDFLAGS)

%.o: %.c
	$(P_CC) -c $<

%.o: %.cpp
	$(P_CXX) -c $<

%.exe: %.o
	$(P_LD) -o $@ $^  $(FINAL_LIBS)

all:
	@echo 'make target'

clean:
	rm -rf *.o *.exe
	@echo clean done

