CC = gcc
CCFLAGS = -g -Wall
DEBUGFLAGS= -D SCANNER_DEBUG
YACC = yacc
LEX = lex
YACCFLAGS = -dv
LEXFLAGS = -l 
YACCLIBS = -ly
LEXLIBS = -ll

mypc: lex.yy.o y.tab.o hash.o node.o tree.o pointerlist.o types.o arg_node.o code.o stack.o
	${CC} ${CCFLAGS} -o mypc lex.yy.o y.tab.o hash.o node.o tree.o pointerlist.o types.o arg_node.o code.o stack.o ${YACCLIBS} ${LEXLIBS}

tree.o: tree.c
	${CC} ${CCFLAGS} -c tree.c

hash.o: hash.c
	${CC} ${CCFLAGS} -c hash.c

node.o: node.c
	${CC} ${CCFLAGS} -c node.c

pointerlist.o: pointerlist.c
	${CC} ${CCFLAGS} -c pointerlist.c

types.o: types.c
	${CC} ${CCFLAGS} -c types.c

arg_node.o: arg_node.c
	${CC} ${CCFLAGS} -c arg_node.c

code.o: code.c
	${CC} ${CCFLAGS} -c code.c

stack.o: stack.c
	${CC} ${CCFLAGS} -c stack.c

y.tab.o: y.tab.c
	${CC} ${CCFLAGS} -c y.tab.c

lex.yy.o: lex.yy.c y.tab.c
	${CC} ${CCFLAGS} -c lex.yy.c

y.tab.c: pc.y
	${YACC} ${YACCFLAGS} pc.y

lex.yy.c: pc.l
	${LEX} ${LEXFLAGS} pc.l

clean:
	rm -f *.o y.tab.* lex.yy.* mypc *~ .*.swp a.out
