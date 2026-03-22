# ============================================================
# Makefile - Super Bulles
# Compile avec Allegro 4 sur Linux / Windows (MinGW)
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lalleg -lm

# Linux : utiliser `allegro-config --cflags --libs` si disponible
# Windows MinGW : adapter le chemin vers allegro

SRCS = src/main.c src/logic.c src/render.c src/ihm.c
OBJS = $(SRCS:.c=.o)
TARGET = superbulles

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
