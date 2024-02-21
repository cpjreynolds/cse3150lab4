CXX=g++
CXXFLAGS=-Wall -g --std=c++20

# testing target
TESTTARGET=lab4test.out
FULLTESTTARGET=lab4fulltest.out
# runnable target
RUNTARGET=lab4.out

# all source files including test
SOURCES:=$(wildcard *.cpp)
OBJECTS:=$(SOURCES:.cpp=.o)

# only the regular main file
#RSOURCES:=$(filter-out %.test.cpp,$(SOURCES))
# only the testing main file
#TSOURCES:=$(filter-out lab2.cpp,$(SOURCES))

.PHONY: all clean check run leaks fullcheck

all: $(RUNTARGET) $(TESTTARGET)

check: $(TESTTARGET)
	./$(TESTTARGET)

run: $(RUNTARGET)
	./$(RUNTARGET)

fullcheck: $(FULLTESTTARGET)
	./$(FULLTESTTARGET)

$(FULLTESTTARGET): $(SOURCES)
	$(CXX) $(CPPFLAGS) -DTESTING -DFULLCHECK $(CXXFLAGS) $^ -o $@

$(TESTTARGET): $(SOURCES)
	$(CXX) $(CPPFLAGS) -DTESTING $(CXXFLAGS) -Wno-unused-function $^ -o $@

$(RUNTARGET): $(SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

leaks: $(RUNTARGET) $(TESTTARGET)
	leaks -atExit -quiet -- ./$(RUNTARGET)
	leaks -atExit -quiet -- ./$(TESTTARGET)

clean:
	rm -rf \
		$(OBJECTS)					\
		$(RUNTARGET)				\
		$(RUNTARGET:.out=.out.dSYM)	\
		$(TESTTARGET)				\
		$(TESTTARGET:.out=.out.dSYM)\
		$(FULLTESTTARGET)			\
		$(FULLTESTTARGET:.out=.out.dSYM)
