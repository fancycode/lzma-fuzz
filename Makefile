ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
SDK_ROOT = $(ROOT)sdk
CORPUS_ROOT = $(ROOT)corpus

SDK_FLAGS = -D_7ZIP_PPMD_SUPPPORT
C_SOURCES = \
	$(SDK_ROOT)/C/7zAlloc.c \
	$(SDK_ROOT)/C/7zArcIn.c \
	$(SDK_ROOT)/C/7zBuf2.c \
	$(SDK_ROOT)/C/7zBuf.c \
	$(SDK_ROOT)/C/7zCrc.c \
	$(SDK_ROOT)/C/7zCrcOpt.c \
	$(SDK_ROOT)/C/7zDec.c \
	$(SDK_ROOT)/C/7zFile.c \
	$(SDK_ROOT)/C/7zStream.c \
	$(SDK_ROOT)/C/Aes.c \
	$(SDK_ROOT)/C/AesOpt.c \
	$(SDK_ROOT)/C/Alloc.c \
	$(SDK_ROOT)/C/Bcj2.c \
	$(SDK_ROOT)/C/Bcj2Enc.c \
	$(SDK_ROOT)/C/Bra86.c \
	$(SDK_ROOT)/C/Bra.c \
	$(SDK_ROOT)/C/BraIA64.c \
	$(SDK_ROOT)/C/CpuArch.c \
	$(SDK_ROOT)/C/Delta.c \
	$(SDK_ROOT)/C/DllSecur.c \
	$(SDK_ROOT)/C/LzFind.c \
	$(SDK_ROOT)/C/Lzma2Dec.c \
	$(SDK_ROOT)/C/Lzma2DecMt.c \
	$(SDK_ROOT)/C/Lzma2Enc.c \
	$(SDK_ROOT)/C/Lzma86Dec.c \
	$(SDK_ROOT)/C/LzmaDec.c \
	$(SDK_ROOT)/C/LzmaEnc.c \
	$(SDK_ROOT)/C/LzmaLib.c \
	$(SDK_ROOT)/C/MtCoder.c \
	$(SDK_ROOT)/C/MtDec.c \
	$(SDK_ROOT)/C/Ppmd7.c \
	$(SDK_ROOT)/C/Ppmd7Dec.c \
	$(SDK_ROOT)/C/Ppmd7Enc.c \
	$(SDK_ROOT)/C/Sha256.c \
	$(SDK_ROOT)/C/Sort.c \
	$(SDK_ROOT)/C/Xz.c \
	$(SDK_ROOT)/C/XzCrc64.c \
	$(SDK_ROOT)/C/XzCrc64Opt.c \
	$(SDK_ROOT)/C/XzDec.c \
	$(SDK_ROOT)/C/XzEnc.c \
	$(SDK_ROOT)/C/XzIn.c

ifeq ($(ENABLE_MT), 1)
	C_SOURCES += \
		$(SDK_ROOT)/C/LzFindMt.c \
		$(SDK_ROOT)/C/Threads.c
else
	SDK_FLAGS += \
		-D_7ZIP_ST=1
endif

C_OBJ = $(C_SOURCES:%.c=%.o)

COMMON_FLAGS=-g -Wall -Werror
INCLUDES = -I$(SDK_ROOT)/C
LIB_FUZZING_ENGINE ?= -lFuzzingEngine

FUZZERS_OBJ = $(patsubst %.cc,%.o,$(wildcard *_fuzzer.cc))
FUZZERS = $(patsubst %.cc,%,$(wildcard *_fuzzer.cc))
CORPUS_DIRS = $(wildcard $(CORPUS_ROOT)/*)
CORPUSES = $(patsubst %,%_seed_corpus.zip,$(notdir $(CORPUS_DIRS)))

LIBRARY = liblzma.a

fuzzers: $(FUZZERS)

%_fuzzer: %_fuzzer.o $(LIBRARY)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(COMMON_FLAGS) -o $@ $(LIB_FUZZING_ENGINE) $+

$(LIBRARY): $(C_OBJ)
	$(AR) r $@ $+

clean:
	rm -f $(LIBRARY) $(C_OBJ)
	rm -f $(FUZZERS_OBJ) $(FUZZERS)
	rm -f $(CORPUSES)

%.o: %.c
	$(CC) $(CFLAGS) $(SDK_FLAGS) $(INCLUDES) $(COMMON_FLAGS) -c -o $@ $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(COMMON_FLAGS) -c -o $@ $<

%_seed_corpus.zip:
	zip -r $@ $(CORPUS_ROOT)/$*

corpuses: $(CORPUSES)

install: fuzzers corpuses
	@[ ! -z "$(DEST)" ] || echo "Run as make DEST=folder"
	@[ ! -z "$(DEST)" ] || exit 1
	cp $(FUZZERS) $(DEST)
	cp $(CORPUSES) $(DEST)
	cp *.dict $(DEST)
