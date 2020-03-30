CC=gcc

IMG_FOLDER=img
EXECUTABLE=DogProgram

EXECUTABLEDIR := bin
SOURCEDIR := src
OBJDIR := obj

SOURCES := $(wildcard $(SOURCEDIR)/*.c)
OBJECTS := $(SOURCES:$(SOURCEDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all prepare clean fullclean

all: prepare $(EXECUTABLE)

prepare:
	-@mkdir $(EXECUTABLEDIR)
	-@mkdir $(IMG_FOLDER)

$(OBJECTS): $(OBJDIR)/%.o: $(SOURCEDIR)/%.c | $(OBJDIR)
	$(CC) -c $< -o $@

$(OBJDIR):
	-@mkdir $(OBJDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $(EXECUTABLEDIR)/$(EXECUTABLE) $^

fullclean: clean
	-rm -R $(EXECUTABLEDIR)
	-rm -R $(OBJDIR)

clean:
	-rm -R $(OBJDIR)
