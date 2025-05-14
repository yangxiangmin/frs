GCC = g++
CP  = cp

TARGET = frssvr
MACHINE ?= $(shell uname -m)

CFLAGS = -g -Wall -Wextra -Wl,-rpath=../libs/ \
         -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -std=c++11 

INCLUDES = -I./ \
	   -I/usr/local/include \
	   -I./include
#           -I/home/tantingxi/awssdk/include

LDFLAGS = -L./ \
          -L/usr/lib/x86_64-linux-gnu \
          -L/usr/local/lib \
          -L./lib64
#          -L./home/tantingxi/awssdk/lib64
          

ifeq ($(uname -m), x86_64) #aarch64
INCLUDES += -I./jsoncpp_x86_64/include 
LDFLAGS  += -L./jsoncpp_x86_64/libs 
else
INCLUDES += -I./jsoncpp_aarch64/include 
LDFLAGS  += -L./jsoncpp_aarch64/libs 
endif



LIBS := -lpthread -lrt -ljsoncpp -laws-cpp-sdk-s3 -laws-cpp-sdk-core

#---------------------------------------------
#
#---------------------------------------------

SOURCES=$(wildcard ./*.cpp) 
OBJS=$(patsubst %.cpp,%.o,$(SOURCES))

$(TARGET) : $(OBJS)
	$(GCC) $^ -o $@  $(INCLUDES) $(LIBS) $(CFLAGS)  $(LDFLAGS)
	
$(OBJS):%.o : %.cpp
	$(GCC) -o $@  $(INCLUDES) $(LIBS) $(CFLAGS) -c $< 
    
.PHONY:clean
clean:
	rm -f $(OBJS) $(TARGET)

    
