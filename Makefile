CC=gcc
CCFLAGS=
%: %.c
	${CC} ${CCFLAGS} $@.c -o bin/$@

all: mchls2ts mc-server-emu
