CC=gcc
FLAGS=-fopenmp -lm

IMG_FOLDER=img
EXECUTABLE=blur-effect

EXECUTABLEDIR := bin
SOURCEDIR := src
OBJDIR := obj

SOURCES := $(wildcard $(SOURCEDIR)/*.c)
OBJECTS := $(SOURCES:$(SOURCEDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all prepare clean fullclean

all: prepare $(EXECUTABLE)

prepare:
	-@mkdir $(EXECUTABLEDIR)

$(OBJECTS): $(OBJDIR)/%.o: $(SOURCEDIR)/%.c | $(OBJDIR)
	$(CC) -Wall -pedantic -c $< -o $@ $(FLAGS)

$(OBJDIR):
	-@mkdir $(OBJDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $(EXECUTABLEDIR)/$(EXECUTABLE) $^

fullclean: clean
	-rm -R $(EXECUTABLEDIR)
	-rm -R $(OBJDIR)

clean:
	-rm -R $(OBJDIR)
