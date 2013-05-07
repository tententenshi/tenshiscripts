CFLAGS = -I/usr/X11R6/include
LIBS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm -lpthread

SRCS	= \
	prog1.c

gltest : $(SRCS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
