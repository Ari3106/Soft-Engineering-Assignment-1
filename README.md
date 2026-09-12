# Faculty Teaching & Office-Hours Scheduler

COSC 369 – Software Engineering I, Project 1

A C program that reads a class-schedule file and a faculty-information file
and automatically generates each instructor's teaching schedule and a
conflict-free weekly office-hours schedule, in the style of the sample
office-hours table from the project handout. The final deliverable is an
**Excel workbook** (`output/faculty_schedule.xlsx`), one sheet per
instructor, produced by a Python helper the Makefile runs after the C
program.

## Project layout

```
include/            Public headers for each module
  course.h            Course struct, day/time parsing & formatting helpers
  faculty.h           Faculty struct, faculty lookup helper
  parser.h            Input-file readers
  scheduler.h          Office-hour generation + report writers

src/                 Module implementations
  course.c, faculty.c, parser.c, scheduler.c   -> built into the static library
  main.c                                       -> the program entry point (not in the library)

data/                Sample input files
  class_schedule.txt   Syllabus / class-schedule input file
  faculty_info.txt     Faculty / office-hours-policy input file

scripts/
  csv_to_excel.py      Converts the generated CSVs into the Excel workbook

lib/, build/, bin/, output/   Created by `make` — not checked in
```

## Building

```
pip install openpyxl   # one-time; the Excel-generation step needs it
make                    # builds lib/libschedule.a, then links bin/scheduler against it
make run                # runs bin/scheduler on the sample data AND builds the .xlsx
make clean              # removes build/, lib/, bin/, output/
```

`make` alone only compiles — nothing is written to `output/` until you run
`make run` (or invoke the binary directly). `make docs` is kept as an alias
for `make run`, since generating the workbook is now the normal end of the
pipeline rather than a separate opt-in step.

## Running

```
./bin/scheduler <class_schedule_file> <faculty_info_file> [output_dir]
```

Example, using the bundled sample data (this is what `make run` does):

```
./bin/scheduler data/class_schedule.txt data/faculty_info.txt output
python3 scripts/csv_to_excel.py output/class_schedule.csv output/office_hours.csv output/faculty_schedule.xlsx
```

This writes:
- `output/schedule_report.txt` — human-readable teaching + office-hours report (plain text)
- `output/class_schedule.csv` — flat CSV of every course section
- `output/office_hours.csv` — flat CSV of every office-hour block
- `output/faculty_schedule.xlsx` — **the final output**: one Excel sheet per
  instructor, each with a CLASS SCHEDULE table and an OFFICE HOURS table,
  built from the two CSVs above by `scripts/csv_to_excel.py`

## Input file formats

Both files are pipe-delimited text. Lines starting with `#` and blank lines
are ignored, so each file is self-documenting — see the comments at the top
of `data/class_schedule.txt` and `data/faculty_info.txt` for the exact field
list. In short:

- **Class schedule** (`CourseCode|Title|Instructor|Days|StartTime|EndTime|Room`)
  is the syllabus data: what each instructor teaches, when, and where.
- **Faculty info** (`Instructor|Department|WeeklyOfficeHours|OfficeRoom|PreferredDays`)
  is the office-hours policy data the lecturer supplies: how many office
  hours per week each faculty member owes, and which days to fall back to
  if their teaching days don't leave enough free time.

You can edit these two files, or point the program at your own files with
the same format, without touching any C code.

## Where the input files live, and why

Both input files are read from the **local filesystem** (plain paths passed
as command-line arguments) rather than fetched over a network. A university
scheduling office runs this on a lab machine against files it already
owns — the syllabus export and the faculty roster are institutional records,
not public data, so there is nothing to fetch from the Internet and no
reason to add network I/O, HTTP parsing, or credentials-handling to a
C program whose job is schedule arithmetic. Reading local files also keeps
the program deterministic and testable (`make run` always produces the same
output from the same two files) and keeps its only external dependency at
the C level down to the standard library (`stdio.h`) — a static library
built for portability and offline use shouldn't require a network stack.

## Static library and Makefile

- **Static library**: `src/course.c`, `src/faculty.c`, `src/parser.c`, and
  `src/scheduler.c` are compiled into object files and archived with
  `ar rcs` into `lib/libschedule.a`. `src/main.c` is compiled separately and
  linked against that archive (`-Llib -lschedule`) to produce `bin/scheduler`.
  See `Makefile` for the exact rules — `$(STATIC_LIB)` depends on the four
  object files, and `$(TARGET)` depends on `main.o` plus the archive, so `make`
  only rebuilds what actually changed.
- **Makefile**: uses pattern rules and order-only prerequisites (`build/`,
  `lib/`, `bin/`, `output/`) so directories are created on demand, and
  expresses the compile → archive → link → run → convert-to-Excel pipeline
  as explicit targets (`all`, `run`, `docs`, `clean`). The Python conversion
  step is a separate rule (invoked by `make run`) rather than something the
  C program does itself, so the C build has no Python dependency and the
  document-generation step can be swapped out independently.

## Office-hours algorithm

For each faculty member, the scheduler looks at the gaps around their own
class times — before their first class of the day, between back-to-back
classes, and after their last class — and fills those gaps with blocks (up
to 2 hours each) until their required weekly office hours are met, never
overlapping a class. If an instructor's teaching days don't leave enough
free time, the scheduler falls back to their `PreferredDays` from the
faculty file to place the remaining hours.

## Regenerating the Excel output

`scripts/csv_to_excel.py <class_schedule.csv> <office_hours.csv> <output.xlsx>`
reads the two CSVs the C program wrote and builds one formatted sheet per
instructor — a CLASS SCHEDULE table followed by an OFFICE HOURS table, both
styled with header shading to match the sample handout (`openpyxl`
required). It's invoked automatically by `make run`; run it by hand on any
matching pair of CSVs if you want to convert a different run's output
without recompiling.
