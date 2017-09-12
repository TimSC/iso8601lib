
all: testiso8601
testiso8601: iso8601.cpp testiso8601.cpp
	g++ $^ -Wall -o $@

