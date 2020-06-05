CC=gcc -std=gnu99
CFLAGS=-g -Wall -Wextra -ggdb3 
LDFLAGS= -lm
EXEC=exec exec_writeCAL exec_readCAL
OBJ=exec.o tools.o bitArray.o integerEncoding.o integerGeneration.o exp.o adjList.o compAdjListG.o compAdjListCL.o compAdjListCB.o compAdjListI.o compAdjList.o
OBJBIS=tools.o bitArray.o integerEncoding.o compAdjListG.o compAdjListCL.o compAdjListCB.o compAdjListI.o compAdjList.o

all: $(EXEC)
	
exec: $(OBJ)
	$(CC) -O3 -o $@ $^ $(LDFLAGS)  
	
exec_writeCAL : exec_writeCAL.o $(OBJBIS)
	$(CC) -O3 -o $@ $^ $(LDFLAGS)

exec_readCAL : exec_readCAL.o $(OBJBIS)
	$(CC) -O3 -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -O3 -o $@ 

clean:
	rm -rf *.o exec exec_writeCAL exec_readCAL
