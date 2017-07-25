FLAGS = -Wall -std=gnu99 -g
DEPENDENCIES = hash.h ftree.h

all: fcopy

fcopy: fcopy.o ftree.o hash_functions.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<

clean:
	cp -rf test/orig_dest/* test/dest
	# cp -rf simple_test/original_dest/* simple_test/dest
	rm -rf simple_test/dest/*
	rm *.o fcopy

test:
	fcopy test/src test/dest

simple_test:
	fcopy simple_test/src simple_test/dest
