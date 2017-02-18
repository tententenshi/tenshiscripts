CFLAGS = -I/usr/X11R6/include
#LIBS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm -lpthread
LIBS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lXi -lXext -lX11 -lm -lpthread

SRC	= \
	prog1.c


gltest : $(SRC)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
