makefile:
clean:
	rm -f *.o
	rm -f *.a

build: 
	cc -c *.c -Wall
	ar -cvq libnpmnetwork.a *.o
	
dist:
	mkdir dist
	mkdir dist/include
	mv libnpmnetwork.a dist/ 
	cp -rf *.h dist/include/

install: build dist clean
	
	
