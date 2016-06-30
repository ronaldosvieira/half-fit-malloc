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

compile2:
	clear
	gcc -c -fpic hfmalloc.c -w -g -O0
	gcc -shared -o libhfmalloc.so hfmalloc.o -g -O0
	gcc test.c -lhfmalloc -L. -o test.o -lm -w -Q -da

clean:
	rm -r *.o *.so