all: repath.so

repath.so: repath.c
	gcc -shared -fPIC repath.c -o repath.so -ldl

install:
	mkdir -p $(DESTDIR)/usr/lib
	cp repath.so $(DESTDIR)/usr/lib

clean:
	rm repath.so
