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
MUDUO_LIB   = libmuduo.a
BIN			= EventLoop_unittest
# EchoServer_unittest EventLoopThreadPool_unittest EventLoopThread_unittest \
 EventLoop_unittest TcpClient_reg1 TcpClient_reg2 TcpClient_reg3 TimerQueue_unittest

INC			= -I$(INC_DIR)
LIB			= -L$(LIB_DIR) -lmuduo -lpthread

all: debug release # strip

debug release gprof: dir $(addprefix $(LIB_DIR)/,$(MUDUO_LIB)) $(addprefix $(BIN_DIR)/,$(BIN))


#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

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

$(BIN_DIR)/EchoServer_unittest:$(addprefix $(OBJ_DIR)/, EchoServer_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/EventLoopThreadPool_unittest:$(addprefix $(OBJ_DIR)/, EventLoopThreadPool_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/EventLoopThread_unittest:$(addprefix $(OBJ_DIR)/, EventLoopThread_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/EventLoop_unittest:$(addprefix $(OBJ_DIR)/, EventLoop_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/TcpClient_reg1:$(addprefix $(OBJ_DIR)/, TcpClient_reg1.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/TcpClient_reg2:$(addprefix $(OBJ_DIR)/, TcpClient_reg2.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/TcpClient_reg3:$(addprefix $(OBJ_DIR)/, TcpClient_reg3.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/TimerQueue_unittest:$(addprefix $(OBJ_DIR)/, TimerQueue_unittest.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -rf  $(OBJ_DIR)
	rm -rf  $(LIB_DIR)
	rm -rf  $(BIN_DIR)

