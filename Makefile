CXX      ?= clang++

CXXFLAGS ?= -O2 -g3 -Werror
CXXFLAGS += -std=c++11 -Wall $(shell sdl-config --cflags) -I.

LDFLAGS  += $(shell sdl-config --libs) -lSDL_image -lSDL_ttf -lGL -lGLEW -lassimp
AR       ?= ar

COMMONDIR = common
COMMONSRCS = $(shell (find $(COMMONDIR) \( -name '*.cpp' -o -name '*.h' \)))

COMMONLIB = $(COMMONDIR)/libcommon.a

LIBSCENESRCDIR = sscene
LIBSCENESRCFILES = Model.cpp HelperFunctions.cpp Scene.cpp
LIBSCENESRCS = $(addprefix $(LIBSCENESRCDIR)/, $(LIBSCENESRCFILES))
LIBSCENEOBJS = $(LIBSCENESRCS:.cpp=.o)
LIBSCENEDEPS = $(LIBSCENESRCS:.cpp=.dep)
LIBSCENELIB = libsscene.a

LIBSCENESHADERFILES = scene.vert scene.frag line.vert line.frag overlay.vert overlay.frag
LIBSCENESHADERDIR = $(LIBSCENESRCDIR)/shaders
LIBSCENESHADERSRCS = $(addprefix $(LIBSCENESHADERDIR)/, $(LIBSCENESHADERFILES))
LIBSCENESHADERS = $(addsuffix .h, $(LIBSCENESHADERSRCS))

INSTALLPREFIX ?= /usr/local

default: all

all: $(LIBSCENELIB) SceneCube

$(COMMONLIB): $(COMMONSRCS)
	make -C $(COMMONDIR)

$(LIBSCENESHADERS): shader.sh $(LIBSCENESHADERSRCS)
	for file in $(LIBSCENESHADERS); do ./shader.sh $$file; done

$(LIBSCENELIB): $(LIBSCENESHADERS) $(LIBSCENEOBJS)
	$(AR) rcs $(LIBSCENELIB) $(LIBSCENEOBJS)

TESTBINDIR = tests/bin

$(TESTBINDIR):
	mkdir -p tests/bin

SceneCube: $(COMMONLIB) $(LIBSCENELIB) $(TESTBINDIR) tests/src/SceneCube.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o tests/bin/SceneCube tests/src/SceneCube.cpp $(LIBSCENELIB) $(COMMONLIB)

install: $(LIBSCENELIB)
	mkdir -p $(INSTALLPREFIX)/include/sscene
	mkdir -p $(INSTALLPREFIX)/lib
	cp -a $(LIBSCENESRCDIR)/*.h $(INSTALLPREFIX)/include/sscene
	cp -a $(LIBSCENELIB) $(INSTALLPREFIX)/lib

%.dep: %.cpp
	@rm -f $@
	@$(CXX) -MM $(CXXFLAGS) $< > $@.P
	@sed 's,\($(notdir $*)\)\.o[ :]*,$(dir $*)\1.o $@ : ,g' < $@.P > $@
	@rm -f $@.P

clean:
	rm -rf SceneCube
	rm -rf common/*.a
	rm -rf common/*.o
	rm -rf sscene/*.o
	rm -rf sscene/*.a
	rm -rf $(LIBSCENESHADERDIR)/*.h
	rm -rf $(LIBSCENELIB)
	rm -rf tests/bin

-include $(LIBSCENEDEPS)

