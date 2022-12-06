files_main = source.o
files_indexer = indexer.o
all : main indexer
main: $(files_main)
	g++ -o main $(files_main) -lsqlite3 
indexer: $(files_indexer)
	g++ -o indexer $(files_indexer) -lsqlite3
source: source.cpp
indexer : indexer.cpp
clean:
	rm -rf *.o main indexer
