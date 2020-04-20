CC=gcc
FLAGS=-fopenmp -lm

IMG_FOLDER=img

EXECUTABLEDIR := bin
OUTPUTDIR := out
SOURCEDIR := src
OBJDIR := obj
RESULTS_DIR := results

SOURCES := $(wildcard $(SOURCEDIR)/*.c)
OBJECTS := $(SOURCES:$(SOURCEDIR)/%.c=$(OBJDIR)/%.o)
EXECUTABLES := $(SOURCES:$(SOURCEDIR)/%.c=$(EXECUTABLEDIR)/%)
RESULTS_SUB_DIRS := $(SOURCES:$(SOURCEDIR)/%.c=$(RESULTS_DIR)/%)
OUT_SUB_DIRS := $(SOURCES:$(SOURCEDIR)/%.c=$(OUTPUTDIR)/%)

.PHONY: all prepare clean fullclean

all: prepare $(EXECUTABLES) clean

prepare:
	-@mkdir $(EXECUTABLEDIR)
	-@mkdir $(OUTPUTDIR)
	-@mkdir $(OUT_SUB_DIRS)
	-@mkdir $(RESULTS_DIR)
	-@mkdir $(RESULTS_SUB_DIRS)

$(OBJECTS): $(OBJDIR)/%.o: $(SOURCEDIR)/%.c | $(OBJDIR)
	$(CC) -Wall -pedantic -c $(FLAGS) $< -o $@

$(OBJDIR):
	-@mkdir $(OBJDIR)

$(EXECUTABLES): $(EXECUTABLEDIR)/%: $(OBJDIR)/%.o | $(EXECUTABLEDIR)
	$(CC) $(FLAGS) $< -o $@

fullclean: clean
	-rm -R $(EXECUTABLEDIR)
	-rm -R $(OBJDIR)
	-rm -R $(RESULTS_DIR)

clean:
	-rm -R $(OBJDIR)
