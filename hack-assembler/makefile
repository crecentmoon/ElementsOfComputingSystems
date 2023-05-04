assembler: assembler.o code.o parser.o symbol_table.o
	gcc -o assembler assembler.o code.o parser.o symbol_table.o

assembler.o: assembler.c code.h parser.h symbol_table.h
	gcc -c assembler.c

code.o: code.c code.h
	gcc -c code.c

parser.o: parser.c parser.h
	gcc -c parser.c

symbol_table.o: symbol_table.c symbol_table.h
	gcc -c symbol_table.c

clean:
	rm -f assembler *.o
