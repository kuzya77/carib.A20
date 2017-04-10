AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES=libblitz.a

libblitz_a_SOURCES=core/debugl.cpp \
core/binfile.cpp core/mbinfile.cpp core/bfsys.cpp core/sbinfile.cpp \
core/memory.cpp core/unzip.c core/stlib.cpp core/misc.cpp core/ioapi.c \
core/picture.cpp core/picturePack.cpp core/gfx.cpp core/screen.cpp core/fscreen.cpp \
menu/menu.cpp menu/console.cpp menu/tconsole.cpp \
anim/anim.cpp anim/animscene.cpp anim/animsys.cpp anim/imagelist.cpp anim/move_controller.cpp anim/static_layer.cpp anim/font.cpp anim/indicator.cpp \
wxf/attr.cpp wxf/parser.cpp wxf/lexer.cpp wxf/wxfpar.cpp wxf/node.cpp \
cmd/cmd.cpp cmd/cmdargs.cpp cmd/utils.cpp

libblitz_a_CPPFLAGS= -Ih -std=c++11

bin_PROGRAMS = torus torus2
torus_SOURCES = test/torus.cpp
torus_LDADD = $(LIBOBJS) libblitz.a -lSDL -lz -lfreeimage
torus_CPPFLAGS = -Ih -std=c++11

torus2_SOURCES = test/torus2.cpp
torus2_LDADD = $(LIBOBJS) libblitz.a -lSDL -lz -lfreeimage
torus2_CPPFLAGS = -Ih -std=c++11
