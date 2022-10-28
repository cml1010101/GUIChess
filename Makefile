GLADE_SRCS = $(wildcard res/*.glade)
GLADE_OBJS = ${GLADE_SRCS:.glade=.o}
PNG_SRCS = $(wildcard res/*.png)
PNG_OBJS = ${PNG_SRCS:.png=.o}
SRCS = $(wildcard src/*.cpp)
OBJS = ${SRCS:.cpp=.o}
deploy: guichess_1.0-1_amd64/usr/bin/gchess guichess_1.0-1_amd64/DEBIAN/control guichess_1.0-1_amd64/usr/share/gchess
	dpkg-deb --build --root-owner-group guichess_1.0-1_amd64
	rm -f -r guichess_1.0-1_amd64
guichess_1.0-1_amd64/usr/bin/gchess:
	mkdir -p guichess_1.0-1_amd64/usr/bin
	cp gchess guichess_1.0-1_amd64/usr/bin/
guichess_1.0-1_amd64/usr/share/gchess:
	mkdir -p guichess_1.0-1_amd64/usr/share
	cp -r res guichess_1.0-1_amd64/usr/share/gchess
guichess_1.0-1_amd64/DEBIAN/control:
	mkdir -p guichess_1.0-1_amd64/DEBIAN
	cp control guichess_1.0-1_amd64/DEBIAN/control
all: ${OBJS} ${GLADE_OBJS} ${PNG_OBJS}
	g++ ${OBJS} ${GLADE_OBJS} ${PNG_OBJS} -o gchess -lchess -g -lboost_program_options `pkg-config gtk+-3.0 --libs` -lpthread -ltorch -lc10 -ltorch_cpu -lboost_regex
	rm -f ${OBJS} ${GLADE_OBJS} ${PNG_OBJS}
clean:
	rm -f -r ${OBJS} ${GLADE_OBJS} ${PNG_OBJS} gchess guichess_1.0-1_amd64
%.o: %.glade
	objcopy -I binary -O elf64-x86-64 $< $@
%.o: %.png
	objcopy -I binary -O elf64-x86-64 $< $@
%.o: %.cpp
	g++ -c $< -o $@ -Iinclude `pkg-config gtk+-3.0 --cflags` -g -I/usr/include/torch/csrc/api/include