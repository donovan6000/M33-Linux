# Cross Platform, Multiple Language Makefile
# By donovan6000

#########################################################################################################################################################################
# Set program specifications

# Program's name
PROGRAM_NAME = M3D_Linux

# Target platform (WINDOWS, LINUX)
TARGET_PLATFORM = LINUX

# Architecture (X32, X64)
ARCHITECTURE = X64

# Programming language (C, C++, ASM)
PROGRAMING_LANGUAGE = C++

# Syntax for ASM (Intel, AT&T)
SYNTAX = Intel

#########################################################################################################################################################################
# Set compiler, flags, libraries, dependencies, and sources

# Windows
ifeq ($(TARGET_PLATFORM), WINDOWS)
	PROG = "$(PROGRAM_NAME).exe"
	ifeq ($(PROGRAMING_LANGUAGE), C)
		ifeq ($(ARCHITECTURE), X32)
			CC = i686-w64-mingw32-gcc
		endif
		ifeq ($(ARCHITECTURE), X64)
			CC = x86_64-w64-mingw32-gcc
		endif
	endif
	ifeq ($(PROGRAMING_LANGUAGE), C++)
		ifeq ($(ARCHITECTURE), X32)
			CC = i686-w64-mingw32-g++
		endif
		ifeq ($(ARCHITECTURE), X64)
			CC = x86_64-w64-mingw32-g++
		endif
	endif
	ifeq ($(PROGRAMING_LANGUAGE), ASM)
		ifeq ($(ARCHITECTURE), X32)
			CC = i686-w64-mingw32-gcc
		endif
		ifeq ($(ARCHITECTURE), X64)
			CC = x86_64-w64-mingw32-gcc
		endif
	endif
endif

# Linux
ifeq ($(TARGET_PLATFORM), LINUX)
	PROG = "$(PROGRAM_NAME)"
	ifeq ($(PROGRAMING_LANGUAGE), C)
		CC = gcc
	endif
	ifeq ($(PROGRAMING_LANGUAGE), C++)
		CC = g++
	endif
	ifeq ($(PROGRAMING_LANGUAGE), ASM)
		CC = gcc
	endif
	ifeq ($(ARCHITECTURE), X32)
		UNIQUE_CFLAGS = -m32
	endif
	ifeq ($(ARCHITECTURE), X64)
		UNIQUE_CFLAGS = -m64
	endif
endif

# Compiling flags
CFLAGS = -Wall -std=c++14 -O3 -static $(UNIQUE_CFLAGS)

ifeq ($(PROGRAMING_LANGUAGE), ASM)
	CFLAGS += -Xassembler -mmnemonic=$(SYNTAX) -Xassembler -msyntax=$(SYNTAX) -Xassembler -mnaked-reg
	ifeq ($(ARCHITECTURE), X32)
		CFLAGS += -Xassembler --32
	endif
	ifeq ($(ARCHITECTURE), X64)
		CFLAGS += -Xassembler --64
	endif
endif

# Global Definitions
CFLAGS += -D$(TARGET_PLATFORM) -D$(ARCHITECTURE)

# Libraries
ifeq ($(TARGET_PLATFORM), WINDOWS)
	LIBS =
endif
ifeq ($(TARGET_PLATFORM), LINUX)
	LIBS =
endif

LIBS +=

# Source files
ifeq ($(PROGRAMING_LANGUAGE), ASM)
	SRCS = *.s
endif
ifeq ($(PROGRAMING_LANGUAGE), C)
	SRCS = *.c
endif
ifeq ($(PROGRAMING_LANGUAGE), C++)
	SRCS = *.cpp
endif

# Dependencies
LIBS +=
CFLAGS +=

#########################################################################################################################################################################
# Make commands

# Default command
all: $(PROG)

$(PROG):   $(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

# Clean command
clean:
	rm -f $(PROG)
	
# Assembly command
asm:	$(SRCS)
	$(CC) $(CFLAGS) -O0 -S -o $(PROG).s $(SRCS) $(LIBS)

# Run command
run:
	./$(PROG)
