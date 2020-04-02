CC=gcc
FLAGS=-lm -lpthread

IMG_FOLDER=img
EXECUTABLE=blur-effect

EXECUTABLEDIR := bin
OUTPUTDIR := out
SOURCEDIR := src
OBJDIR := obj

SOURCES := $(wildcard $(SOURCEDIR)/*.c)
OBJECTS := $(SOURCES:$(SOURCEDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all prepare clean fullclean

all: prepare $(EXECUTABLE) clean

prepare:
	-@mkdir $(EXECUTABLEDIR)
	-@mkdir $(OUTPUTDIR)

$(OBJECTS): $(OBJDIR)/%.o: $(SOURCEDIR)/%.c | $(OBJDIR)
	$(CC) -Wall -pedantic -c $< -o $@

$(OBJDIR):
	-@mkdir $(OBJDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -pg -o $(EXECUTABLEDIR)/$(EXECUTABLE) $^ $(FLAGS)

fullclean: clean
	-rm -R $(EXECUTABLEDIR)
	-rm -R $(OBJDIR)

clean:
	-rm -R $(OBJDIR)
