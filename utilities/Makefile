#
# Makefile for utilities library
#
#########################################

include ../this_dir.mk
include ../options.mk

#########################################

HEADERS=array2.h intarray2.h hash.h indent.h cputime.h tarray1.h input.h error.h minmax.h

OBJECTS= intarray2.o error.o ran1.o\
	indexx.o hash.o cputime.o tarray1.o input.o

SOURCES= array2.cc error.cc ran1.cc indexx.cc hash.cc\
	cputime.cc tarray1.cc testarray.cc intarray2.cc input.cc

CCFLAGS= -I$(INCLUDEDIR) $(OPTIMIZATIONS)
CCGFLAGS= -I$(INCLUDEDIR) -g -DBOUNDS
CCPGFLAGS= -I$(INCLUDEDIR) -pg -O

.SUFFIXES: .cc $(SUFFIXES)

.cc.o:	$*.cc
	$(CCCOM) -c $(CCFLAGS) $<

install: headerscopy libutilities.a libutilities-g.a libutilities-pg.a 
	cp libutilities.a libutilities-g.a libutilities-pg.a $(LIBDIR)
	ranlib $(LIBDIR)/libutilities.a
	ranlib $(LIBDIR)/libutilities-g.a
	ranlib $(LIBDIR)/libutilities-pg.a

libutilities.a:	$(OBJECTS) $(HEADERS)
	ar rv libutilities.a $(OBJECTS)
	ranlib libutilities.a

libutilities-g.a: $(SOURCES) $(HEADERS)
	$(CCCOM) $(CCGFLAGS) -c intarray2.cc
	$(CCCOM) $(CCGFLAGS) -c tarray1.cc
	$(CCCOM) $(CCGFLAGS) -c error.cc
	$(CCCOM) $(CCGFLAGS) -c ran1.cc
	$(CCCOM) $(CCGFLAGS) -c indexx.cc
	$(CCCOM) $(CCGFLAGS) -c hash.cc
	$(CCCOM) $(CCGFLAGS) -c cputime.cc
	$(CCCOM) $(CCGFLAGS) -c input.cc
	ar rv libutilities-g.a $(OBJECTS)
	ranlib libutilities-g.a

libutilities-pg.a: $(SOURCES) $(HEADERS)
	$(CCCOM) $(CCPGFLAGS) -c intarray2.cc
	$(CCCOM) $(CCPGFLAGS) -c tarray1.cc
	$(CCCOM) $(CCPGFLAGS) -c error.cc
	$(CCCOM) $(CCPGFLAGS) -c ran1.cc
	$(CCCOM) $(CCPGFLAGS) -c indexx.cc
	$(CCCOM) $(CCPGFLAGS) -c hash.cc
	$(CCCOM) $(CCPGFLAGS) -c cputime.cc
	$(CCCOM) $(CCPGFLAGS) -c input.cc
	ar rv libutilities-pg.a $(OBJECTS)
	ranlib libutilities-pg.a

headerscopy:	
	cp $(HEADERS) $(INCLUDEDIR)

clean:	
	rm -f *.o libutilities.a libutilities-g.a libutilities-pg.a 
