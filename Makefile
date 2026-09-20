#
# Makefile för bildjämförelse
#

CXX       := g++ -std=c++20 -pipe
OPTIMIZE  := -O3
FLAGS     := $(OPTIMIZE) -Wall -Wextra -g
LDFLAGS   := -pthread
LIBFLAGS  := -lsfml-graphics -lsfml-window -lsfml-system
SFML_EXEC := slow fast
HASH_EXEC := hash_test
EXEC      := $(HASH_EXEC) $(SFML_EXEC)
MAKEDEPS  := -MMD -MP

-include extra.mk

OBJECTS   := $(patsubst %.cpp,build/%.o,$(wildcard *.cpp))
DEPS      := $(patsubst %.o,%.d,$(OBJECTS))
SHAREDOBJECTS := $(filter-out $(addprefix build/,$(addsuffix .%,$(EXEC))),$(OBJECTS))

$(shell mkdir -p build)

.PHONY: all clean zap
all: $(EXEC)

$(SFML_EXEC): %: build/%.o $(SHAREDOBJECTS)
	$(CXX) $(FLAGS) -o $@ $^ $(LDFLAGS) $(LIBFLAGS)

$(HASH_EXEC): %: build/%.o
	$(CXX) $(FLAGS) -o $@ $^ $(LDFLAGS)

$(OBJECTS): build/%.o: %.cpp
	$(CXX) $(FLAGS) $(MAKEDEPS) -c -o $@ $<

clean:
	@ \rm -rf build/

zap: clean
	@ \rm -f $(EXEC) *~

-include $(DEPS)
