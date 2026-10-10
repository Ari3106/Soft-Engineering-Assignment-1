# ============================================================================
# Makefile - COSC 369 Project 1: Faculty Teaching & Office-Hours Scheduler
#
# Builds src/course.c, src/faculty.c, src/parser.c and src/scheduler.c into a
# static library (lib/libschedule.a), then compiles src/main.c and links it
# against that library to produce bin/scheduler.
#
# make run:
#   Runs the C scheduler and creates the CSV, text, and Excel outputs.
#
# make knowledge:
#   Runs the scheduler and generates the Prolog knowledge base.
#
# make ask:
#   Generates the knowledge base and starts the interactive question system.
#
# make demo:
#   Generates the knowledge base and runs automatic demonstration questions.
# ============================================================================

CC := cc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude -g
AR := ar
ARFLAGS := rcs
PYTHON ?= python

SRC_DIR := src
BUILD_DIR := build
LIB_DIR := lib
BIN_DIR := bin
DATA_DIR := data
OUT_DIR := output
SCRIPT_DIR := scripts
PROLOG_DIR := prolog

LIB_SOURCES := $(SRC_DIR)/course.c $(SRC_DIR)/faculty.c $(SRC_DIR)/parser.c $(SRC_DIR)/scheduler.c
LIB_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SOURCES))

MAIN_SOURCE := $(SRC_DIR)/main.c
MAIN_OBJECT := $(BUILD_DIR)/main.o

STATIC_LIB := $(LIB_DIR)/libschedule.a
TARGET := $(BIN_DIR)/scheduler

CLASS_FILE := $(DATA_DIR)/class_schedule.txt
FACULTY_FILE := $(DATA_DIR)/faculty_info.txt

SYLLABUS_FACTS := $(DATA_DIR)/syllabi/appolo_tankeh_syllabus_facts.txt
GENERATED_FACTS := $(PROLOG_DIR)/generated_facts.pl
RULES_FILE := $(PROLOG_DIR)/rules.pl

.PHONY: all run docs knowledge ask demo clean


# --- build everything --------------------------------------------------------

all: $(TARGET)


# --- static library ----------------------------------------------------------

$(STATIC_LIB): $(LIB_OBJECTS) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJECTS)


# --- final executable, linked against the static library --------------------

$(TARGET): $(MAIN_OBJECT) $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJECT) -L$(LIB_DIR) -lschedule


# --- object files ------------------------------------------------------------

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


# --- directories -------------------------------------------------------------

$(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(OUT_DIR):
	mkdir -p $@


# --- run original scheduler and create Excel output --------------------------

run: $(TARGET) | $(OUT_DIR)
	./$(TARGET) $(CLASS_FILE) $(FACULTY_FILE) $(OUT_DIR)
	$(PYTHON) $(SCRIPT_DIR)/csv_to_excel.py \
		$(OUT_DIR)/class_schedule.csv \
		$(OUT_DIR)/office_hours.csv \
		$(OUT_DIR)/faculty_schedule.xlsx


# --- create Prolog knowledge base -------------------------------------------

knowledge: run
	$(PYTHON) $(SCRIPT_DIR)/build_knowledge_base.py \
		$(OUT_DIR)/class_schedule.csv \
		$(OUT_DIR)/office_hours.csv \
		$(FACULTY_FILE) \
		$(SYLLABUS_FACTS) \
		$(GENERATED_FACTS)


# --- interactive question system --------------------------------------------

ask: knowledge
	$(PYTHON) $(SCRIPT_DIR)/ask_questions.py


# --- automatic question demonstration ---------------------------------------

demo: knowledge
	$(PYTHON) $(SCRIPT_DIR)/ask_questions.py --demo


# --- alias: make docs is the same as make run -------------------------------

docs: run


# --- remove generated build/output files ------------------------------------

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(OUT_DIR)
	rm -f $(GENERATED_FACTS)