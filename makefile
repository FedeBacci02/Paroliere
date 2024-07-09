.PHONY: clean

# Compiler
CC = cc

# Compiler flags
CFLAGS = -Wall -g -pedantic
LDFLAGS = -lpthread

# Headers 
HDRS = 	./librerie/CodaThreads.h \
 		./librerie/listaConcatenataPlayer.h \
		./librerie/listaUser.h \
		./librerie/macros.h \
		./librerie/matrix.h \
		./librerie/messageStructure.h \
		./librerie/stringFunction.h \
		./librerie/Trie.h 

# Source files for libraries
SRCLIBS = ./librerie/CodaThreads.c \
		  ./librerie/listaConcatenataPlayer.c \
		  ./librerie/listaUser.c \
		  ./librerie/matrix.c \
		  ./librerie/Trie.c \
		  ./librerie/messageStructure.c


# Object files for libraries
OBJLIBS = ./CodaThreads.o \
		  ./listaConcatenataPlayer.o \
		  ./listaUser.o \
		  ./matrix.o \
		  ./Trie.o \
		  ./messageStructure.o

# Executable files
EXE1 = paroliere_cl
EXE2 = paroliere_sv

# Object files
OBJS1 = paroliere_cl.o $(OBJLIBS)
OBJS2 = paroliere_sv.o $(OBJLIBS)

# Source files
SRCS1 = paroliere_cl.c
SRCS2 = paroliere_sv.c

# Targets
all: $(EXE1) $(EXE2)

$(EXE1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(EXE1) $(OBJS1) $(LDFLAGS)

$(EXE2): $(OBJS2)
	$(CC) $(CFLAGS) -o $(EXE2) $(OBJS2) $(LDFLAGS)

paroliere_cl.o: $(SRCS1) $(HDRS)
	$(CC) $(CFLAGS) -c $(SRCS1)

paroliere_sv.o: $(SRCS2) $(HDRS)
	$(CC) $(CFLAGS) -c $(SRCS2)

CodaThreads.o: ./librerie/CodaThreads.c ./librerie/CodaThreads.h
	$(CC) $(CFLAGS) -c ./librerie/CodaThreads.c

listaConcatenataPlayer.o: ./librerie/listaConcatenataPlayer.c ./librerie/listaConcatenataPlayer.h
	$(CC) $(CFLAGS) -c ./librerie/listaConcatenataPlayer.c

listaUser.o: ./librerie/listaUser.c ./librerie/listaUser.h
	$(CC) $(CFLAGS) -c ./librerie/listaUser.c

matrix.o: ./librerie/matrix.c ./librerie/matrix.h
	$(CC) $(CFLAGS) -c ./librerie/matrix.c

Trie.o: ./librerie/Trie.c ./librerie/Trie.h
	$(CC) $(CFLAGS) -c ./librerie/Trie.c

messageStructure.o: ./librerie/messageStructure.c ./librerie/messageStructure.h
	$(CC) $(CFLAGS) -c ./librerie/messageStructure.c


clean:
	rm -f $(OBJS1) $(OBJS2) $(EXE1) $(EXE2)
	
# End of Makefile