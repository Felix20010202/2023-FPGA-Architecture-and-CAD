CXX      := g++
CXXFLAGS := -std=c++11 -O3 -Wall -Wextra -I../include
EXT_OBJS := $(wildcard ../obj/*.o)
EXEC     := ../bin/legalizer
SRCS     := Legalizer.cpp main.cpp FPGA.cpp DetailedPlacer.cpp
OBJS     := $(SRCS:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJS) $(EXT_OBJS)
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

ifeq (test, $(firstword $(MAKECMDGOALS)))
  TESTCASE := $(word 2, $(MAKECMDGOALS))
  $(eval $(TESTCASE):;@:)
endif

test: $(EXEC)
	@echo test on ../input/$(TESTCASE)/architecture.txt ../input/$(TESTCASE)/instance.txt ../input/$(TESTCASE)/netlist.txt
	$(EXEC) ../input/$(TESTCASE)/architecture.txt ../input/$(TESTCASE)/instance.txt ../input/$(TESTCASE)/netlist.txt ../output/$(TESTCASE).txt
	./../verifier/verifier2 ../input/$(TESTCASE)/architecture.txt ../input/$(TESTCASE)/instance.txt ../input/$(TESTCASE)/netlist.txt ../output/$(TESTCASE).txt

EXAMPLE := ../bin/example

example: $(EXAMPLE)

$(EXAMPLE): Example.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(EXT_OBJS)

clean:
	rm -f $(EXEC) $(OBJS) $(EXAMPLE)

.PHONY: all test example clean
