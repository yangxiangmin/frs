GCC = g++
CP  = cp

TARGET = frssvr
MACHINE ?= $(shell uname -m)

# 输出目录定义
BUILD_DIR = buildout

CFLAGS = -g -Wall -Wextra -Wl,-rpath=../libs/ \
         -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -std=c++11 

# 头文件路径
INCLUDES = -Iinclude \
           -I/usr/local/include

# 库文件路径
LDFLAGS = -L/usr/lib/x86_64-linux-gnu \
          -L/usr/local/lib \
          -Llib64

# 架构特定配置
ifeq ($(MACHINE), x86_64)
INCLUDES += -Ijsoncpp_x86_64/include
LDFLAGS  += -Ljsoncpp_x86_64/libs
else
INCLUDES += -Ijsoncpp_aarch64/include
LDFLAGS  += -Ljsoncpp_aarch64/libs
endif

LIBS := -lpthread -lrt -ljsoncpp -laws-cpp-sdk-s3 -laws-cpp-sdk-core

#---------------------------------------------
# 源文件和目标文件定义
#---------------------------------------------
SOURCES = $(wildcard src/*.cpp)          # 源文件在src目录
OBJS    = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))  # 目标文件在buildout

# 主目标：确保构建目录存在
$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(@D)  # 确保输出目录存在
	$(GCC) $^ -o $@ $(INCLUDES) $(LIBS) $(CFLAGS) $(LDFLAGS)

# 编译规则
$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(@D)  # 确保目标目录存在
	$(GCC) -c $< -o $@ $(INCLUDES) $(CFLAGS)

# 清理规则
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/$(TARGET)