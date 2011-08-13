PROGRAM = spooky-maze
SOURCES = src/game.c src/graphics.c src/input.c src/levels.c \
          src/player.c src/zombie.c
OBJECTS = $(SOURCES:.c=.o)

INCS = `sdl-config --cflags` -Iinclude
LIBS = `sdl-config --libs` -lSDL_image

all: $(PROGRAM)
	
$(PROGRAM): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(LIBS) $(OBJECTS) -o $@

.c.o:
	$(CC) -g $(CFLAGS) $(INCS) -c $< -o $@

install:
	install -d $(DESTDIR)/usr/bin
	install -m 0755 $(PROGRAM) $(DESTDIR)/usr/bin

clean:
	rm -f $(PROGRAM) $(OBJECTS)
