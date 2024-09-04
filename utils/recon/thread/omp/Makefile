# cd utils/recon/thread/omp/
# make & make all

CXX = g++
CXXFLAGS = -fopenmp -O3 -Wall

TARGET = main
SRCS = add_mat_omp.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)