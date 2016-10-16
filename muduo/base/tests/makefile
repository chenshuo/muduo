# makefile for muduo/base/tests

.SUFFIXES: .sh .h .c .cc .cpp

AR          = ar
AR_FLAGS    = crs

CXX			= g++
CXXFLAGS	= -std=c++11 -ggdb3 -Wall -Werror
CXX_MACRO	=

BASE_DIR    = ../../../
INC_DIR		= $(BASE_DIR)
SRC_DIR		= ./
OBJ_DIR		= obj
LIB_DIR		= lib
BIN_DIR		= bin

MUDUO_INC   = -I$(INC_DIR)
MUDUO_LIB   = -L$(LIB_DIR) -lmuduo_base -lpthread

GTEST_INC   = -I$(BASE_DIR)/muduo/other/
GTEST_LIB   = -L$(BASE_DIR)/muduo/other/gtest/lib/ -lgtest

ZLIB_INC    = -I$(BASE_DIR)/muduo/other/zlib/inc
ZLIB_LIB    = -L$(BASE_DIR)/muduo/other/zlib/lib -lz

INC			= $(MUDUO_INC) $(GTEST_INC) $(ZLIB_INC)
LIB			= $(MUDUO_LIB) $(GTEST_LIB) $(ZLIB_LIB)

MUDUOLIB    = libmuduo_base.a
BIN			= BoundedBlockingQueue_test
# AsyncLogging_test Atomic_unittest BlockingQueue_bench BlockingQueue_test BoundedBlockingQueue_test \
 Date_unittest Exception_test FileUtil_test Fork_test LogFile_test LogStream_test \
 LogStream_bench Logging_test Mutex_test ProcessInfo_test Singleton_test \
 SingletonThreadLocal_test ThreadLocalSingleton_test ThreadLocal_test \
 ThreadPool_test Thread_bench Thread_test TimeZone_unittest Timestamp_unittest \
 GzipFile_test

all: debug release # strip

debug release gprof: dir $(addprefix $(LIB_DIR)/,$(MUDUOLIB)) $(addprefix $(BIN_DIR)/,$(BIN))


#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(BASE_DIR)/muduo/base/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(LIB_DIR)/libmuduo_base.a:$(addprefix $(OBJ_DIR)/, \
	Thread.o ThreadPool.o CountDownLatch.o Timestamp.o LogFile.o Logging.o \
	FileUtil.o LogStream.o TimeZone.o Date.o ProcessInfo.o Exception.o \
	AsyncLogging.o Timestamp.o Condition.o)
	$(AR) $(AR_FLAGS) $@ $^

$(BIN_DIR)/Atomic_unittest:$(addprefix $(OBJ_DIR)/, Atomic_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^

$(BIN_DIR)/AsyncLogging_test:$(addprefix $(OBJ_DIR)/, AsyncLogging_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/BlockingQueue_bench:$(addprefix $(OBJ_DIR)/, BlockingQueue_bench.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/BlockingQueue_test:$(addprefix $(OBJ_DIR)/, BlockingQueue_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/BoundedBlockingQueue_test:$(addprefix $(OBJ_DIR)/, BoundedBlockingQueue_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Date_unittest:$(addprefix $(OBJ_DIR)/, Date_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Exception_test:$(addprefix $(OBJ_DIR)/, Exception_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/FileUtil_test:$(addprefix $(OBJ_DIR)/, FileUtil_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Fork_test:$(addprefix $(OBJ_DIR)/, Fork_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/GzipFile_test:$(addprefix $(OBJ_DIR)/, GzipFile_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/LogFile_test:$(addprefix $(OBJ_DIR)/, LogFile_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/LogStream_bench:$(addprefix $(OBJ_DIR)/, LogStream_bench.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Logging_test:$(addprefix $(OBJ_DIR)/, Logging_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Mutex_test:$(addprefix $(OBJ_DIR)/, Mutex_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/ProcessInfo_test:$(addprefix $(OBJ_DIR)/, ProcessInfo_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Singleton_test:$(addprefix $(OBJ_DIR)/, Singleton_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/SingletonThreadLocal_test:$(addprefix $(OBJ_DIR)/, SingletonThreadLocal_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/ThreadLocalSingleton_test:$(addprefix $(OBJ_DIR)/, ThreadLocalSingleton_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/ThreadLocal_test:$(addprefix $(OBJ_DIR)/, ThreadLocal_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/ThreadPool_test:$(addprefix $(OBJ_DIR)/, ThreadPool_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Thread_bench:$(addprefix $(OBJ_DIR)/, Thread_bench.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Thread_test:$(addprefix $(OBJ_DIR)/, Thread_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/TimeZone_unittest:$(addprefix $(OBJ_DIR)/, TimeZone_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/Timestamp_unittest:$(addprefix $(OBJ_DIR)/, Timestamp_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/LogStream_test:$(addprefix $(OBJ_DIR)/, LogStream_test.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -rf  $(OBJ_DIR)
	rm -rf  $(LIB_DIR)
	rm -rf  $(BIN_DIR)

