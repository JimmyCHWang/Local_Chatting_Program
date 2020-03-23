
# Makefile for CSE156 (W20) Final Project (myclient/myserver)
# by Jingheng Wang (jwang280@ucsc.edu)

FLAGS = -Wall -pthread -lreadline
# Public Sources
SOURCES_P = src/cse156.h src/cse156.cpp src/final_prot.h src/final_prot.cpp

# Client Sources
SOURCES_C = src/myclient.cpp
EXEBIN_C = bin/myclient

# Server Sources
SOURCES_S = src/myserver.cpp
EXEBIN_S = bin/myserver

# Submit Sources
TAR_FILES = bin doc src Makefile

all:
	g++ -o $(EXEBIN_C) $(SOURCES_P) $(SOURCES_C) $(FLAGS)
	g++ -o $(EXEBIN_S) $(SOURCES_P) $(SOURCES_S) $(FLAGS)

client:
	g++ -o $(EXEBIN_C) $(SOURCES_P) $(SOURCES_C) $(FLAGS)

server:
	g++ -o $(EXEBIN_S) $(SOURCES_P) $(SOURCES_S) $(FLAGS)

tar:
	tar -czvf jwang280chatproj.tar.gz $(TAR_FILES)

gzip:
	tar -czvf jwang280chatproj.tar.gz $(TAR_FILES)

clean :
	rm -f $(EXEBIN_S) $(EXEBIN_C)

