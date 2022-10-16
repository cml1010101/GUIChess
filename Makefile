GLADE_SRCS = $(wildcard res/*.glade)
GLADE_OBJS = ${GLADE_SRCS:.glade=.o}
PNG_SRCS = $(wildcard res/*.png)
PNG_OBJS = ${PNG_SRCS:.png=.o}
SRCS = $(wildcard src/*.cpp)
OBJS = ${SRCS:.cpp=.o}
all: ${OBJS} ${GLADE_OBJS} ${PNG_OBJS}
	g++ ${OBJS} ${GLADE_OBJS} ${PNG_OBJS} -o gchess -lchess -g -lboost_program_options `pkg-config gtk+-3.0 --libs` -lpthread
	rm -f ${OBJS} ${GLADE_OBJS} ${PNG_OBJS}
clean:
	rm -f ${OBJS} ${GLADE_OBJS} ${PNG_OBJS} gchess
%.o: %.glade
	objcopy -I binary -O elf64-x86-64 $< $@
%.o: %.png
	objcopy -I binary -O elf64-x86-64 $< $@
%.o: %.cpp
	g++ -c $< -o $@ -Iinclude `pkg-config gtk+-3.0 --cflags` -g