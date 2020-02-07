CXX=g++
CXXFLAGS=-std=c++14 $(shell pkg-config --libs --cflags opencv) -fopenmp
SRCDIR=src
SRCS=$(shell find $(SRCDIR) -maxdepth 1 -name '*.cpp')
OBJDIR=objs
OBJS=$(subst $(SRCDIR),$(OBJDIR), $(SRCS))
OBJS:=$(subst .cpp,.o,$(OBJS))
TARGET=img2color

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $+ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $< -c -o $@

clean:
	rm -rf $(OBJS)
	rm -rf $(TARGET)
