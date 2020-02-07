CXX=g++
CXXFLAGS=-std=c++14 $(shell pkg-config --libs --cflags opencv)
OMPFLAGS=-fopenmp
NVCC=nvcc
NVCCFLAGS=$(CXXFLAGS) --compiler-bindir=$(CXX) -Xcompiler=$(OMPFLAGS) -gencode arch=compute_60,code=sm_60
SRCDIR=src
SRCS=$(shell find $(SRCDIR) -maxdepth 1 -name '*.cu' -o -name '*.cpp')
OBJDIR=objs
OBJS=$(subst $(SRCDIR),$(OBJDIR), $(SRCS))
OBJS:=$(subst .cpp,.o,$(OBJS))
OBJS:=$(subst .cu,.o,$(OBJS))
HEADERS=$(shell find $(SRCDIR) -name '*.cuh' -o -name '*.hpp' -o -name '*.h')
TARGET=img2color

$(TARGET): $(OBJS)
	$(NVCC) $(NVCCFLAGS) $+ -o $@

$(SRCDIR)/%.cpp: $(SRCDIR)/%.cu $(HEADERS)
	$(NVCC) $(NVCCFLAGS) --cuda $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $< -c -o $@

clean:
	rm -rf $(OBJS)
	rm -rf $(TARGET)
