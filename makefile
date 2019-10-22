SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .cpp .hpp .o

RM = rm -f
CPP=g++

SRCDIR=.
ALL_CFLAGS=
CPPFLAGS=

ifdef DEBUG
	CFLAGS = -g -DDEBUG
	O=Debug
else
	CFLAGS = -O2 -DNDEBUG
	O=Release
endif


LFLAGS=
DEFINES = 
INCLUDES = -I$(SRCDIR) -I/usr/include
LIBS =

$O/%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -o $O/$*.o $*.cpp

DOCFILES = lang/en/lf1.xml lang/fr/lf1.xml
HTMLFILES = $O/lang/en/lf.html $O/lang/fr/lf.html
MANFILES = $O/lang/en/lf.1 $O/lang/fr/lf.1
TARGET = $O/lf
LANGS = en fr
OBJS = $O/lf.o $O/filetest.o $O/util.o $O/wordexp.o


.PHONY: all manfiles htmlfiles clean distclean

all::	$(TARGET) $(MANFILES) $(HTMLFILES)

$(TARGET): $O $(OBJS)
	$(CPP) -o $@ $(LFLAGS) $(OBJS) $(LIBS)
ifndef DEBUG
	strip $(TARGET)
endif

lf.hpp: lang/??/lf_strings.hpp

$O/lf.o: lf.cpp lf.hpp

htmlfiles: $(HTMLFILES)

$O/lang/%/lf.html: lang/%/lf1.xml
	xmlto --skip-validation -o $(@D) html $<
	mv $(@D)/index.html $@

manfiles: $(MANFILES)

$O/lang/%/lf.1: lang/%/lf1.xml
	xmlto --skip-validation -o $(@D) man $<

tags: $(SRCS)
	ctags -d --globals --members -T --language=c++ *.cpp *.hpp

Debug:
	mkdir Debug Debug/lang Debug/lang/en Debug/lang/fr

Release:
	mkdir Release Release/lang Release/lang/en Release/lang/fr

clean:
	$(RM) $O/core $O/*.m $O/*.o $(TARGET) $O/lang/??/*.1 $O/lang/??/*.html

distclean: clean
	$(RM) -f tags make.m
	$(RM) -fr Debug Release
