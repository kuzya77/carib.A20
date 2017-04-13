bin_PROGRAMS = carib

carib_SOURCES = game.cpp common.cpp terminal.cpp head.cpp h_math.cpp h_mcomby.cpp h_rnd.cpp h_math_storage.cpp h_box.cpp h_common.cpp hs_waita.cpp hs_gameb.cpp hs_game.cpp hs_gamew.cpp

carib_LDADD = $(LIBOBJS) ../engine/libblitz.a -lSDL -lz -lfreeimage -lSDL_mixer

carib_CPPFLAGS = -I../engine/h -std=c++11
