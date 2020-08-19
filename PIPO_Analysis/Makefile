


OUTPUT = PIPO_Analysis.out
CC     = gcc
CC_OPT = -m64 -O3 -std=gnu11

SRCPATH = .
INCPATH = .

SRCS   = $(SRCPATH)/dif_trail_searching.c
OBJS   = dif_trail_searching.o
SRCS  += $(SRCPATH)/global.c
OBJS  += global.o
SRCS  += $(SRCPATH)/lin_trail_searching.c
OBJS  += lin_trail_searching.o
SRCS  += $(SRCPATH)/prep.c
OBJS  += prep.o
SRCS  += $(SRCPATH)/searching_main.c
OBJS  += searching_main.o
SRCS  += $(SRCPATH)/utils.c
OBJS  += utils.o


all : $(OUTPUT)


$(OUTPUT) : $(OBJS)
	$(CC) $(CC_OPT) -o $(OUTPUT) $(OBJS) -lm

$(OBJS): $(SRCS)
	$(CC) $(CC_OPT) -c -w $(SRCS) -I $(INCPATH) -lm



clean :
	rm -rf $(OBJS) $(OUTPUT)