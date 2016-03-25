# Makefile
# Copyright (C) 2016 pexeer@gamil.com
# Fri Mar 25 07:15:46 UTC 2016

STD=-std=c++11
WARN=-Wall -Werror
DEBUG=-g -ggdb
#OPT=-O3

GTEST_LIB=../../p-3rd/gtest/lib/libgtest.a
GTEST_HEADER=../../p-3rd/gtest/include/

FINAL_CFLAGS=$(WARN) $(OPT) $(DEBUG) $(CFLAGS)
FINAL_CXXFLAGS=$(STD) $(WARN) $(OPT) $(DEBUG) $(CFLAGS) -I./../include -I$(GTEST_HEADER)
FINAL_LDFLAGS=$(LDFLAGS)  $(DEBUG)
FINAL_LIBS=-lm -pthread

CCCOLOR="\033[34m"
LINKCOLOR="\033[34;1m"
SRCCOLOR="\033[33m"
BINCOLOR="\033[37;1m"
MAKECOLOR="\033[32;1m"
ENDCOLOR="\033[0m"

QUIET_C = @printf '    %b %b\n' $(CCCOLOR)CXX$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_LINK = @printf '%b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;

P_CC=$(QUIET_C)$(CC) $(FINAL_CFLAGS)
P_CXX=$(QUIET_LINK)$(CXX) $(FINAL_CXXFLAGS)

all: utils_test.ut

%.ut: %.cpp
	$(P_CXX) -o $@ $^  $(GTEST_LIB) $(FINAL_LIBS)

clean:
	rm -rf *.ut
