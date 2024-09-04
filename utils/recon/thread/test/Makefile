# cd utils/recon/thread/test/
# make & make all

CXX = gcc
CXXFLAGS = -fopenmp

TARGET = test_omp
SRCS = test_omp.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)