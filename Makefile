
all: testiso8601
testiso8601: iso8601.c testiso8601.c
	gcc -fPIC $^ -lm -Wall -o $@

