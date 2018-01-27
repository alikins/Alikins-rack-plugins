SLUG = Alikins
VERSION = 0.5.1

PLUGIN_SDK ?= ../..
# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES = $(wildcard src/*.cpp)


# Must include the VCV plugin Makefile framework
include $(PLUGIN_SDK)/plugin.mk

# http://cppcheck.sourceforge.net/
cppcheck:
	cppcheck -i../../dep/include -i../../include --enable=style -DVERSION=0.5.1 --quiet src/

# https://github.com/google/styleguide
cpplint:
	cpplint --headers=hpp --filter=-whitespace/line_length,-legal/copyright,whitespace/blank_line src/*.cpp src/*.hpp

# Convenience target for packaging files into a ZIP file
dist: all
	mkdir -p dist/$(SLUG)
	cp LICENSE* dist/$(SLUG)/
	cp $(TARGET) dist/$(SLUG)/
	cp -R res dist/$(SLUG)/
	cd dist && zip -5 -r $(SLUG)-$(VERSION)-$(ARCH).zip $(SLUG)

.PHONY: cppcheck cpplint dist
