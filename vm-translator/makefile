.PHONY: translator clean

translator: main.o parser.o code_writer.o
	gcc -o translator main.o parser.o code_writer.o

main.o: parser.h code_writer.h
	gcc -c main.c

parser.o: parser.h
	gcc -c parser.c

parser_private.o: parser.h
	gcc -c parser_private.c 

code_writer.o: code_writer.h parser.h
	gcc -c code_writer.c

code_writer_private.o: code_writer.h
	gcc -c code_writer_private.c

clean:
	rm -f translator *.o *.asm