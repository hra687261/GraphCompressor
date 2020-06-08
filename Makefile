CC=gcc -std=gnu99
CCE=$(CC) -O3 -o
CFLAGS= -g -Wall -Wextra
LDFLAGS=-lm
EXEC=exec writeAL readAL writeCAL readCAL
OBJ=exec.o tools.o bitArray.o integerEncoding.o integerGeneration.o exp.o adjList.o compAdjListG.o compAdjListCL.o compAdjListCB.o compAdjListI.o compAdjList.o
OBJBIS=tools.o bitArray.o integerEncoding.o compAdjListG.o compAdjListCL.o compAdjListCB.o compAdjListI.o compAdjList.o

all: $(EXEC)
	
exec: $(OBJ)
	$(CCE) $@ $^ $(LDFLAGS)  
	
writeAL : writeAL.o $(OBJBIS)
	$(CCE) $@ $^ $(LDFLAGS)

readAL : readAL.o $(OBJBIS)
	$(CCE) $@ $^ $(LDFLAGS)

writeCAL : writeCAL.o $(OBJBIS)
	$(CCE) $@ $^ $(LDFLAGS)

readCAL : readCAL.o $(OBJBIS)
	$(CCE) $@ $^ $(LDFLAGS)


%.o: %.c
	$(CC) $(CFLAGS) -c $< -O3 -o $@ 

clean:
	rm -rf *.o exec writeAL readAL writeCAL readCAL
