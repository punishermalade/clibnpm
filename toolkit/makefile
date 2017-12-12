makefile:
clean:
	rm -f *.o
	rm -f *.a

build:
	cc -c *.c -Wall
	ar -cvq toolkit.a *.o
	
dist:
	mkdir dist
	rm -rf dist/*
	mkdir dist/include/
	rm -rf dist/include/*
	mv -f toolkit.a dist/
	cp -rf *.h dist/include/

install: build dist clean
	
