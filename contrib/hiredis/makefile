# makefile for muduo/net/tests

.SUFFIXES: .sh .h .c .cc .cpp

AR          = ar
AR_FLAGS    = crs

CXX			= g++
CXXFLAGS	= -std=c++11 -ggdb3 -Wall -Werror
CXX_MACRO	=

BASE_DIR    = ../../
INC_DIR		= $(BASE_DIR)
SRC_DIR		= ./
OBJ_DIR		= obj
LIB_DIR		= lib
BIN_DIR		= bin
MUDUOLIB    = libmuduo.a

MUDUO_INC   = -I$(INC_DIR)
MUDUO_LIB   = -L$(LIB_DIR) -lmuduo -lpthread

GTEST_INC   = -I$(BASE_DIR)/muduo/other/
GTEST_LIB   = -L$(BASE_DIR)/muduo/other/gtest/lib/ -lgtest

ZLIB_INC    = -I$(BASE_DIR)/muduo/other/zlib/inc
ZLIB_LIB    = -L$(BASE_DIR)/muduo/other/zlib/lib -lz

HIREDIS_INC = -I$(BASE_DIR)/muduo/other/hiredis/
HIREDIS_LIB = -L$(BASE_DIR)/muduo/other/hiredis/ -lhiredis

INC			= $(MUDUO_INC) $(GTEST_INC) $(ZLIB_INC) $(HIREDIS_INC)
LIB			= $(MUDUO_LIB) $(GTEST_LIB) $(ZLIB_LIB) $(HIREDIS_LIB)

BIN			= mrediscli

all: debug release # strip

debug release gprof: hiredis dir $(addprefix $(LIB_DIR)/,$(MUDUOLIB)) $(addprefix $(BIN_DIR)/,$(BIN))


#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

hiredis:
	@-cd $(BASE_DIR)/muduo/other/hiredis && make static

$(OBJ_DIR)/%.o:$(BASE_DIR)/muduo/base/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(BASE_DIR)/muduo/net/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(BASE_DIR)/muduo/net/poller/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(LIB_DIR)/libmuduo.a:$(addprefix $(OBJ_DIR)/, \
	Thread.o ThreadPool.o CountDownLatch.o Timestamp.o LogFile.o Logging.o \
	FileUtil.o LogStream.o TimeZone.o Date.o ProcessInfo.o Exception.o \
	AsyncLogging.o Timestamp.o Condition.o EventLoop.o TcpServer.o \
	InetAddress.o TcpConnection.o TimerQueue.o Channel.o Socket.o \
	Poller.o DefaultPoller.o Acceptor.o EventLoopThreadPool.o SocketsOps.o \
	Timer.o PollPoller.o EPollPoller.o EventLoopThread.o Buffer.o \
	TcpClient.o Connector.o)
	$(AR) $(AR_FLAGS) $@ $^

$(BIN_DIR)/mrediscli:$(addprefix $(OBJ_DIR)/, Hiredis.o mrediscli.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -rf  $(OBJ_DIR)
	rm -rf  $(LIB_DIR)
	rm -rf  $(BIN_DIR)
	cd $(BASE_DIR)/muduo/other/hiredis && make clean

