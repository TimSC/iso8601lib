all: testiso8601
testiso8601: iso8601.c testiso8601.c
	gcc -fPIC $^ -lm -Wall -o $@

# Fuzzing (needs clang with libFuzzer). Runs for FUZZ_TIME seconds, e.g.
#   make fuzz FUZZ_TIME=600
# Inputs the fuzzer discovers are kept in fuzz/corpus/ between runs.
# Crashing inputs are written to fuzz/crash-*.
FUZZ_TIME ?= 60
FUZZ_CC ?= clang

fuzz/fuzz_iso8601: iso8601.c fuzz/fuzz_iso8601.c iso8601.h
	$(FUZZ_CC) -g -O1 -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined \
		-I. iso8601.c fuzz/fuzz_iso8601.c -lm -o $@

fuzz: fuzz/fuzz_iso8601
	mkdir -p fuzz/corpus
	./fuzz/fuzz_iso8601 -max_total_time=$(FUZZ_TIME) -max_len=128 \
		-artifact_prefix=fuzz/ fuzz/corpus fuzz/seeds

.PHONY: all fuzz
