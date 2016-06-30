all:
	clear
	gcc -c -fpic hfmalloc.c -w
	gcc -shared -o libhfmalloc.so hfmalloc.o
	gcc test.c -lhfmalloc -L. -o test.o -lm -w
	./test.o

compile:
	clear
	gcc -c -fpic hfmalloc.c -w
	gcc -shared -o libhfmalloc.so hfmalloc.o
	gcc test.c -lhfmalloc -L. -o test.o -lm -w

clean:
	rm -r *.o *.so