CC=gcc -std=gnu99
O=-O3 -o
CFLAGS=-g -Wall -Wextra
LDFLAGS=#-lm
EXEC=renumbering writeAL readAL writeCAL readCAL_BFS readCAL_PR
ALOBJS=tools.o integerEncoding.o compAdjListG.o compAdjListCL.o compAdjListCB.o compAdjListI.o
CALOBJS=tools.o integerEncoding.o compAdjList.o
all: $(EXEC)
	
renumbering : renumbering.o tools.o
	$(CC) $(O) $@ $^ $(LDFLAGS)
	
writeAL : writeAL.o $(ALOBJS) 
	$(CC) $(O) $@ $^ $(LDFLAGS)

readAL : readAL.o $(ALOBJS)
	$(CC) $(O) $@ $^ $(LDFLAGS)

writeCAL : writeCAL.o $(CALOBJS)
	$(CC) $(O) $@ $^ $(LDFLAGS)

readCAL_BFS : readCAL_BFS.o $(CALOBJS)
	$(CC) $(O) $@ $^ $(LDFLAGS)

readCAL_PR: readCAL_PR.o $(CALOBJS)
	$(CC) $(O) $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(O) $@ 

clean:
	rm -rf *.o renumbering writeAL readAL writeCAL readCAL_BFS readCAL_PR