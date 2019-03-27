ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
SDK_ROOT = $(ROOT)sdk
CORPUS_ROOT = $(ROOT)corpus

C_SOURCES = \
	$(SDK_ROOT)/C/7zCrc.c \
	$(SDK_ROOT)/C/7zCrcOpt.c \
	$(SDK_ROOT)/C/CpuArch.c \
	$(SDK_ROOT)/C/Delta.c \
	$(SDK_ROOT)/C/Lzma2Dec.c \
	$(SDK_ROOT)/C/LzmaDec.c

C_OBJ = $(C_SOURCES:%.c=%.o)

INCLUDES = -I$(SDK_ROOT)/C
LIB_FUZZING_ENGINE ?= -lFuzzingEngine

FUZZERS_OBJ = $(patsubst %.cc,%.o,$(wildcard *_fuzzer.cc))
FUZZERS = $(patsubst %.cc,%,$(wildcard *_fuzzer.cc))
CORPUS_DIRS = $(wildcard $(CORPUS_ROOT)/*)
CORPUSES = $(patsubst %,%_seed_corpus.zip,$(notdir $(CORPUS_DIRS)))

LIBRARY = liblzma.a

fuzzers: $(FUZZERS)

%_fuzzer: %_fuzzer.o $(LIBRARY)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -g -o $@ $+ $(LIB_FUZZING_ENGINE)

$(LIBRARY): $(C_OBJ)
	$(AR) r $@ $+

clean:
	rm -f $(LIBRARY) $(C_OBJ)
	rm -f $(FUZZERS_OBJ) $(FUZZERS)
	rm -f $(CORPUSES)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -g -c -o $@ $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -g -c -o $@ $<

%_seed_corpus.zip:
	zip -r $@ $(CORPUS_ROOT)/$*

corpuses: $(CORPUSES)

install: fuzzers corpuses
	@[ ! -z "$(DEST)" ] || echo "Run as make DEST=folder"
	@[ ! -z "$(DEST)" ] || exit 1
	cp $(FUZZERS) $(DEST)
	cp $(CORPUSES) $(DEST)
