CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g
TARGET   := simulator
SRCDIR   := src
BUILDDIR := build

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCS))

.PHONY: all clean run week4

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: all
	./$(TARGET)

week4: all
	bash experiments/run_week4.sh

clean:
	rm -rf $(BUILDDIR) $(TARGET)
