# Target: HP PA-RISC running linux
TDEPFILES= pa-tdep.o pa-linux-tdep.o solib.o solib-svr4.o solib-legacy.o
TM_FILE= tm-linux.h

GDBSERVER_DEPFILES= low-linux.o
