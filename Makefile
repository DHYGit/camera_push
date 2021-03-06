
include ../Rules.mk

APP := ./bin/camera_push

ARGUS_UTILS_DIR := $(TOP_DIR)/argus/samples/utils

SRCS := \
	./src/main.cpp \
	./src/ConsumerThreadTool.cpp \
	./src/LibRtmpTool.cpp \
	./src/rtmp_push_librtmp.cpp \
        ./src/libpcm_aac.cpp \
        ./src/Raspberry_Pi_Record.cpp \
	$(wildcard $(CLASS_DIR)/*.cpp) \
	$(ARGUS_UTILS_DIR)/Thread.cpp

OBJS := $(SRCS:.cpp=.o)

CPPFLAGS += \
	-I"$(ARGUS_UTILS_DIR)"\
        -I ./include

LDFLAGS += \
	-lnveglstream_camconsumer -largus \
        -lrtmp \
        -lm -lz \
        -lpulse \
        -lfaac -lasound \
        -lpulse-simple 

all: $(APP)

$(CLASS_DIR)/%.o: $(CLASS_DIR)/%.cpp
	$(AT)$(MAKE) -C $(CLASS_DIR)

%.o: %.cpp
	@echo "Compiling: $<"
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(APP): $(OBJS)
	@echo "Linking: $@"
	$(CPP) -o $@ $(OBJS) $(CPPFLAGS) $(LDFLAGS)

clean:
	$(AT)rm -rf $(APP) $(OBJS)
