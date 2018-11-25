TARGET=optimal_action
SOURCE=optimal_action.cpp
SOURCE_R=optimal_action.R

CXX=g++
RSCRIPT=Rscript
CPPFLAGS=-std=gnu++14 -O2 -Wall
LIBPATH=
LDFLAGS=-lboost_program_options

LOGFILE=optimal_action.csv
RUNTIME_OPTIONS_LOG=--trials 1 --softmax 1 --samples 1000001 --logfile $(LOGFILE)
RUNTIME_OPTIONS_SHORT_1=--trials 2 --samples 1000003 --learning_rate 0.00001045 --exploration_ratio 0.1067
RUNTIME_OPTIONS_SHORT_2=--trials 3 --samples 1000004 --learning_rate 0.00001056 --exploration_ratio 0.1078 --softmax 1
RUNTIME_OPTIONS_LONG_1=--trials 10
RUNTIME_OPTIONS_LONG_2=--trials 10 --softmax 1

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) $(LIBPATH) -o $@ $(SOURCE) $(LDFLAGS) $(LIBS)

run: $(TARGET)
	./$(TARGET) $(RUNTIME_OPTIONS_LOG)
	$(RSCRIPT) $(SOURCE_R)
	./$(TARGET) $(RUNTIME_OPTIONS_SHORT_1)
	./$(TARGET) $(RUNTIME_OPTIONS_SHORT_2)
	./$(TARGET) $(RUNTIME_OPTIONS_LONG_1)
	./$(TARGET) $(RUNTIME_OPTIONS_LONG_2)

clean:
	rm -f $(TARGET)
