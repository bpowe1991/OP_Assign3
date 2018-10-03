default: oss user

oss.o: oss.c
	gcc -c oss.c -o oss.o

oss: oss.o
	gcc oss.o -o oss

user.o: user.c
	gcc -c user.c -o user.o

user: user.o
	gcc user.o -o user

clean:
	-rm -f *.o
	-rm -f oss
	-rm -f user
	-rm -f *.out