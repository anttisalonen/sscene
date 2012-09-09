CXX      = clang++
CXXFLAGS = -std=c++11 -Wall -Werror $(shell sdl-config --cflags) -O2 -I.
LDFLAGS  = $(shell sdl-config --libs) -lSDL_image -lSDL_ttf -lGL -lGLEW -lassimp
AR       = ar

COMMONDIR = common
COMMONSRCS = $(shell (find $(COMMONDIR) \( -name '*.cpp' -o -name '*.h' \)))

COMMONLIB = $(COMMONDIR)/libcommon.a

LIBSCENESRCDIR = sscene
LIBSCENESRCFILES = Model.cpp HelperFunctions.cpp Scene.cpp
LIBSCENESRCS = $(addprefix $(LIBSCENESRCDIR)/, $(LIBSCENESRCFILES))
LIBSCENEOBJS = $(LIBSCENESRCS:.cpp=.o)
LIBSCENELIB = libsscene.a

LIBSCENESHADERFILES = scene.vert scene.frag
LIBSCENESHADERDIR = $(LIBSCENESRCDIR)/shaders
LIBSCENESHADERSRCS = $(addprefix $(LIBSCENESHADERDIR)/, $(LIBSCENESHADERFILES))
LIBSCENESHADERS = $(addsuffix .h, $(LIBSCENESHADERSRCS))

default: $(LIBSCENELIB)

all: $(LIBSCENELIB) SceneCube

$(COMMONLIB): $(COMMONSRCS)
	make -C $(COMMONDIR)

$(LIBSCENESHADERS):
	for file in $(LIBSCENESHADERS); do ./shader.sh $$file; done

$(LIBSCENELIB): $(LIBSCENESHADERS) $(LIBSCENEOBJS)
	$(AR) rcs $(LIBSCENELIB) $(LIBSCENEOBJS)

TESTBINDIR = tests/bin

$(TESTBINDIR):
	mkdir -p tests/bin

SceneCube: $(COMMONLIB) $(LIBSCENELIB) $(TESTBINDIR) tests/src/SceneCube.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o tests/bin/SceneCube tests/src/SceneCube.cpp $(LIBSCENELIB) $(COMMONLIB)

clean:
	rm -rf SceneCube
	rm -rf common/*.a
	rm -rf common/*.o
	rm -rf src/*.o
	rm -rf src/*.a
	rm -rf $(LIBSCENESHADERDIR)/*.h
	rm -rf $(LIBSCENELIB)
	rm -rf tests/bin

