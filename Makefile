# ============================================================================
# Makefile - COSC 369 Project 1: Faculty Teaching & Office-Hours Scheduler
#
# Builds src/course.c, src/faculty.c, src/parser.c and src/scheduler.c into a
# static library (lib/libschedule.a), then compiles src/main.c and links it
# against that library to produce bin/scheduler. `make run` executes the
# program against the two sample input files in data/ (writing a text report
# and two CSVs to output/), then calls the Python helper to turn those CSVs
# into the final Excel workbook, output/faculty_schedule.xlsx.
# ============================================================================

CC      := cc
CFLAGS  := -Wall -Wextra -std=c11 -Iinclude -g
AR      := ar
ARFLAGS := rcs
PYTHON  := python3

SRC_DIR    := src
BUILD_DIR  := build
LIB_DIR    := lib
BIN_DIR    := bin
DATA_DIR   := data
OUT_DIR    := output
SCRIPT_DIR := scripts

LIB_SOURCES := $(SRC_DIR)/course.c $(SRC_DIR)/faculty.c $(SRC_DIR)/parser.c $(SRC_DIR)/scheduler.c
LIB_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SOURCES))

MAIN_SOURCE := $(SRC_DIR)/main.c
MAIN_OBJECT := $(BUILD_DIR)/main.o

STATIC_LIB := $(LIB_DIR)/libschedule.a
TARGET     := $(BIN_DIR)/scheduler

CLASS_FILE   := $(DATA_DIR)/class_schedule.txt
FACULTY_FILE := $(DATA_DIR)/faculty_info.txt

.PHONY: all run docs clean

all: $(TARGET)

# --- static library -------------------------------------------------------
$(STATIC_LIB): $(LIB_OBJECTS) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJECTS)

# --- final executable, linked against the static library ------------------
$(TARGET): $(MAIN_OBJECT) $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJECT) -L$(LIB_DIR) -lschedule

# --- object files -----------------------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# --- directories ------------------------------------------------------------
$(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(OUT_DIR):
	mkdir -p $@

# --- run the program against the sample inputs, then build the Excel output -
run: $(TARGET) | $(OUT_DIR)
	./$(TARGET) $(CLASS_FILE) $(FACULTY_FILE) $(OUT_DIR)
	$(PYTHON) $(SCRIPT_DIR)/csv_to_excel.py \
		$(OUT_DIR)/class_schedule.csv $(OUT_DIR)/office_hours.csv $(OUT_DIR)/faculty_schedule.xlsx

# --- alias kept for convenience: `make docs` is the same as `make run` ------
docs: run

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(OUT_DIR)
