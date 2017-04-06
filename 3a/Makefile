
test: test.c libmem.so mem.h 
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. 
	gcc -o test test.c -Wall -L. -lmem  


libmem.so: mem.so
	gcc -shared -o libmem.so mem.o

mem.so: mem.c
	gcc -c -fpic mem.c -Wall -Werror

clean:
	-rm libmem.so mem.o test



	
