CC = gcc -fPIC -g
all: libmylibc.so libmyfs.so test

test: test.o libmylibc.so libmyfs.so
	$(CC) test.o ./libmylibc.so ./libmyfs.so -o test

libmyfs.so: fs.o
	$(CC) fs.o -shared -o libmyfs.so

libmylibc.so: mylibc.o
	$(CC) mylibc.o -shared -o libmylibc.so

mylibc.o:
	$(CC) -c mylibc.c

%.o: %.c
	$(CC) -c $< -o $@ 

clean:
	rm -f *.o test *.so

.PHONY: all run clean
