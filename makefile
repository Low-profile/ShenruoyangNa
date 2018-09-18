parser: lex.yy.c parser.cpp
	g++ parser.cpp lex.yy.c -o parser 

lex.yy.c: scanner.l scanner.h
	flex scanner.l

clean:
	rm -f lex.yy.c parser