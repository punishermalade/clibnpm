makefile:
clean:
	rm -f *.o
	rm -f *.a

build: 
	cc -c internlog.c client.c server.c -Wall
	ar -cvq network.a *.o
	
dist:
	mkdir dist
	rm -rf dist/*
	mkdir dist/include
	rm -rf dist/include/*
	mv -f network.a dist/ 
	cp -rf *.h dist/include/

install: build dist clean
