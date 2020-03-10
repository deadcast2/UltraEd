include $(ROOT)\usr\include\make\PRdefs

N64KITDIR = $(ROOT)\..\nintendo\n64kit
NUSYSINCDIR = $(N64KITDIR)\nusys\include
NUSYSLIBDIR = $(N64KITDIR)\nusys\lib
NUSTDINC = $(N64KITDIR)\nustd\include
NUSTDLIB = $(N64KITDIR)\nustd\lib
LIB = $(ROOT)\usr\lib
LPR = $(LIB)\PR
INC = $(ROOT)\usr\include
64DRIVEUSB = 64drive_usb
LCDEFS = -DNU_DEBUG -DF3DEX_GBI_2
LCINCS = -I. -I$(NUSTDINC) -I$(NUSYSINCDIR) -I$(ROOT)\usr\include\PR -I$(ROOT)\GCC\MIPSE\INCLUDE
LCOPTS = -G 0
LDFLAGS = $(MKDEPOPT) -L$(LIB) -L$(NUSYSLIBDIR) -lnualsgi_d -lnusys -lgultra_rom -L$(NUSTDLIB) -lnustd -L$(ROOT)\GCC\MIPSE\LIB -lkmc
OPTIMIZER =	-g
APP = main.out
TARGETS = main.n64
CODEFILES = main.c utilities.c upng.c actor.c collision.c
CODEOBJECTS = $(CODEFILES:.c=.o)  $(NUSYSLIBDIR)\nusys.o
DATAOBJECTS = $(DATAFILES:.c=.o)
CODESEGMENT = codesegment.o
OBJECTS = $(CODESEGMENT) $(DATAOBJECTS)

default: $(TARGETS)

include $(COMMONRULES)

.PHONY: load

load:
	$(64DRIVEUSB) -l $(TARGETS)

$(CODESEGMENT):	$(CODEOBJECTS) Makefile
	$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(TARGETS):	$(OBJECTS)
	$(MAKEROM) spec -s 9 -I$(NUSYSINCDIR) -r $(TARGETS) -e $(APP)
	makemask $(TARGETS)
