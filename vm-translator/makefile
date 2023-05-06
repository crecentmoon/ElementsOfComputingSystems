.PHONY: translator clean

translator: main.o parser.o code_writer.o
	gcc -o translator main.o parser.o code_writer.o

main.o: parser.h code_writer.h
	gcc -c main.c

parser.o: parser.h parser_private.h
	gcc -c parser.c

code_writer.o: code_writer.h code_writer_private.h
	gcc -c code_writer.c

clean:
	rm -f translator *.o *.asm