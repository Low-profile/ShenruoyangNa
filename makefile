SOURCES = $(wildcard ./passing-tests/*.lil)

parser: lex.yy.c parser.cpp
	g++ -std=c++11 parser.cpp lex.yy.c -o parser 


lex.yy.c: scanner.l scanner.h
	flex scanner.l

clean:
	rm -f lex.yy.c parser

run:
	for file in ${SOURCES} ; do \
		./parser $$file  ; \
	done