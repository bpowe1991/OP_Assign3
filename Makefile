default: oss

oss.o: oss.c
	gcc -c oss.c -o oss.o

oss: oss.o
	gcc oss.o -o oss

clean:
	-rm -f *.o
	-rm -f oss
	-rm -f Worker
	-rm -f *.out