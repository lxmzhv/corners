CC=g++

# Windows options
#CC=mingw32-g++
MINGW_FLAGS=#-I/mingw/include
MINGW_LIBS=#-L/mingw/lib

#CC=i586-mingw32msvc-g++
#MINGW_FLAGS=-I/usr/i586-mingw32msvc/include
#MINGW_LIBS=-L/usr/i586-mingw32msvc/lib

DEBUG=-g -O2 -pg
#DEBUG=-O2
CFLAGS=`pkg-config --cflags gtk+-2.0` -Iinclude -Iinclude_lcore $(MINGW_FLAGS) $(DEBUG)
LFLAGS=`pkg-config --libs gtk+-2.0` $(MINGW_LIBS)

CPP_FILES=cpp/* cpp_lcore/*
H_FILES=include/* include_lcore/*
SRC_FILES=$(CPP_FILES) $(H_FILES)
OBJ_FILES=obj/ai.o obj/ui.o obj/directions.o obj/experience.o obj/field.o obj/game.o obj/utils.o obj/strings.o obj/numbers.o obj/draw.o obj/about.o obj/preferences.o
INSTALL_DIR=/usr/local

release: all
	strip corners

all: corners messages

rebuild: clean all

run: all tags
	./corners

messages: locale/ru/LC_MESSAGES/corners.mo locale/en/LC_MESSAGES/corners.mo
	
locale/ru/LC_MESSAGES/corners.mo: po/ru.po
	msgfmt -o $@ po/ru.po

locale/en/LC_MESSAGES/corners.mo: po/en.po
	msgfmt -o $@ po/en.po

new_messages: cpp/ui.cpp
	xgettext -C -k_ -o po/id.po cpp/*
	msgmerge -U po/ru.po po/id.po
	msgmerge -U po/en.po po/id.po
	gvim -d "po/ru.po~" po/ru.po 

tags: $(SRC_FILES)
	ctags --c-kinds=+px $(SRC_FILES)

tags-legacy: $(SRC_FILES)
	ctags --defines --typedefs --typedefs-and-c++ $(SRC_FILES)

corners: $(OBJ_FILES)
	$(CC) obj/*.o $(LFLAGS) -o corners

obj/ai.o:	cpp/ai.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/ai.cpp

obj/ui.o:	cpp/ui.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/ui.cpp

obj/directions.o:	cpp/directions.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/directions.cpp

obj/draw.o:	cpp/draw.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/draw.cpp

obj/about.o:	cpp/about.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/about.cpp

obj/preferences.o:	cpp/preferences.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/preferences.cpp

obj/experience.o:	cpp/experience.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/experience.cpp

obj/field.o:	cpp/field.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/field.cpp

obj/game.o:	cpp/game.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/game.cpp

obj/utils.o:	cpp/utils.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp/utils.cpp

obj/strings.o:	cpp_lcore/strings.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp_lcore/strings.cpp

obj/numbers.o:	cpp_lcore/numbers.cpp $(H_FILES)
	$(CC) -c $(CFLAGS) -o $@ cpp_lcore/numbers.cpp

#	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f corners obj/*.o tags

install: release
	rm -fr "$(INSTALL_DIR)/share/corners/"
	rm -f "$(INSTALL_DIR)/bin/corners"
	mkdir -p "$(INSTALL_DIR)/share/corners"
	cp -r corners locale corners.png corners-no-bg.png "$(INSTALL_DIR)/share/corners/"
	echo "cd '$(INSTALL_DIR)/share/corners/' && ./corners" > "$(INSTALL_DIR)/bin/corners"
	chmod a+x "$(INSTALL_DIR)/bin/corners"
