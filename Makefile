RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
# FLAGS +=
FLAGS += -w -g
CFLAGS +=
CXXFLAGS += -Werror

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
# SOURCES = $(wildcard src/*.cpp)
SOURCES = src/alikins.cpp \
		  src/IdleSwitch.cpp \
#		  src/BigMuteButton.cpp \
#		  src/GateLength.cpp \
		  src/MomentaryOnButtons.cpp \
		  src/Reference.cpp \
		  src/ValueSaver.cpp
		 # src/ShiftPedal.cpp \
		 # src/ColorPanel.cpp
		 # src/SpecificValue.cpp


# Must include the VCV plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# http://cppcheck.sourceforge.net/
cppcheck:
	cppcheck -i$(RACK_DIR)/dep/include -i$(RACK_DIR)/include --enable=style -DVERSION=0.5.1 --quiet src/

# https://github.com/google/styleguide
cpplint:
	cpplint --headers=hpp --filter=-whitespace/line_length,-legal/copyright,-whitespace/blank_line src/*.cpp src/*.hpp


DISTRIBUTABLES += $(wildcard LICENSE*) res

.PHONY: cppcheck cpplint
