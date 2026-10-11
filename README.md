# Faculty Teaching & Office-Hours Scheduler

COSC 369 – Software Engineering I, Project 1

A C program that reads a class-schedule file and a faculty-information file
and automatically generates each instructor's teaching schedule and a
conflict-free weekly office-hours schedule, in the style of the sample
office-hours table from the project handout. The final deliverable is an
**Excel workbook** (`output/faculty_schedule.xlsx`), one sheet per
instructor, produced by a Python helper the Makefile runs after the C
program.

Every push to GitLab runs a CI/CD pipeline (`.gitlab-ci.yml`) that builds
the program, generates the workbook, and runs the Prolog demo — see
[Continuous Integration (GitLab CI/CD)](#continuous-integration-gitlab-cicd).

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

On macOS and Linux, where the interpreter is called `python3`, pass it to
make: `make run PYTHON=python3` (Windows: `make run PYTHON=py`).

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

---

# Prolog Knowledge and Question System

## Overview

The original project is a C-based faculty teaching and office-hours scheduler. It reads faculty and class information, generates teaching and office-hour schedules, produces CSV output files, and creates an Excel workbook.

The project has been extended with a Python, PySwip, and SWI-Prolog knowledge system.

The knowledge system allows a user to ask questions about:

- Courses
- Faculty members
- Teaching assignments
- Faculty office hours
- Office locations
- Departments
- Available syllabus information
- Course prerequisites
- Course policies
- Course descriptions
- Course events and dates
- Course meeting times

Python converts the scheduler output and available syllabus information into Prolog facts. PySwip connects the Python question interface to SWI-Prolog so that Prolog can use facts and rules to answer questions.
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# Requirements

The project requires the following software:

- C compiler
- Make
- Python 3
- SWI-Prolog
- PySwip
- openpyxl

The Python dependencies are listed in:

```text
requirements.txt
```

The file contains:

```text
openpyxl
pyswip
```

Install the Python dependencies with:

```bash
python -m pip install -r requirements.txt
```

On Windows, the Python launcher can also be used:

```bash
py -m pip install -r requirements.txt
```

---

# SWI-Prolog Requirement

SWI-Prolog must be installed separately because PySwip communicates with SWI-Prolog.

Python and SWI-Prolog should use matching architectures. For example:

```text
Python       64-bit
SWI-Prolog   64-bit
```

SWI-Prolog can be checked with:

```bash
swipl --version
```

Python architecture can be checked with:

```bash
python -c "import struct; print(struct.calcsize('P') * 8)"
```

A successful PySwip connection can be tested with:

```bash
python -c "from pyswip import Prolog; p=Prolog(); print(list(p.query('X = 5')))"
```

A successful result should look similar to:

```text
[{'X': 5}]
```

---

# Optional Windows Virtual Environment

A Python virtual environment is recommended on Windows so that the project uses the correct Python installation.

This project was tested with Python 3.12 64-bit.

Create the virtual environment from the project root:

```powershell
py -3.12 -m venv .venv
```

Activate it in PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
```

When activated, the terminal should begin with:

```text
(.venv)
```

Check the Python version:

```powershell
python --version
```

Check which Python is being used:

```powershell
python -c "import sys; print(sys.executable)"
```

The path should point to:

```text
.venv\Scripts\python.exe
```

Install the requirements:

```powershell
python -m pip install -r requirements.txt
```

The `.venv` directory should not be committed to Git.

---

# Project Structure

Important project files and folders include:

```text
Soft-Engineering-Assignment-1/
│
├── data/
│   ├── class_schedule.txt
│   ├── faculty_info.txt
│   └── syllabi/
│       ├── Appolo_Tankeh_COSC369_Syllabus.docx
│       └── appolo_tankeh_syllabus_facts.txt
│
├── include/
│   ├── course.h
│   ├── faculty.h
│   ├── parser.h
│   └── scheduler.h
│
├── src/
│   ├── course.c
│   ├── faculty.c
│   ├── parser.c
│   ├── scheduler.c
│   └── main.c
│
├── scripts/
│   ├── csv_to_excel.py
│   ├── build_knowledge_base.py
│   └── ask_questions.py
│
├── prolog/
│   ├── rules.pl
│   └── generated_facts.pl
│
├── output/
│   ├── class_schedule.csv
│   ├── office_hours.csv
│   ├── schedule_report.txt
│   └── faculty_schedule.xlsx
│
├── Makefile
├── requirements.txt
├── README.md
├── .gitlab-ci.yml
└── .gitignore
```

---

# Original C Program Inputs

The original C scheduler continues to use exactly two input files:

```text
data/class_schedule.txt
data/faculty_info.txt
```

The syllabus information is not a third input to the C scheduler.

Instead, syllabus information is used by the Python and Prolog knowledge-system extension after the original C scheduler creates its output files.

---

# Generated Files

The C scheduler generates:

```text
output/class_schedule.csv
output/office_hours.csv
output/schedule_report.txt
```

The Python Excel helper generates:

```text
output/faculty_schedule.xlsx
```

The Prolog knowledge-base builder generates:

```text
prolog/generated_facts.pl
```

`generated_facts.pl` is generated automatically and should not be edited manually.

If the source data or syllabus facts change, regenerate the file using:

```bash
make knowledge
```

or on Windows:

```bash
make knowledge PYTHON=py
```

---

# How the Knowledge System Works

The complete flow of the project is:

```text
class_schedule.txt
faculty_info.txt
        |
        v
    C Scheduler
        |
        +-------------------------+
        |                         |
        v                         v
class_schedule.csv        office_hours.csv
        |                         |
        +------------+------------+
                     |
                     v
          build_knowledge_base.py
                     |
              + syllabus facts
                     |
                     v
          generated_facts.pl
                     +
                 rules.pl
                     |
                     v
                SWI-Prolog
                     |
                     v
                  PySwip
                     |
                     v
           ask_questions.py
                     |
                     v
             User Questions
```

The original C code remains responsible for:

- Reading the class schedule
- Reading faculty information
- Generating faculty teaching schedules
- Generating office-hour schedules
- Creating the schedule report
- Creating CSV output

Python and Prolog extend the original program by creating a searchable knowledge base.

---

# Available Syllabus Information

The current project includes syllabus information for:

```text
COSC 369 - Software Engineering I
Dr. Appolo Tankeh
```

The syllabus information is stored in:

```text
data/syllabi/appolo_tankeh_syllabus_facts.txt
```

Only available syllabus information is added to the knowledge base.

The project does not create or assume syllabus information for faculty members or courses for which a syllabus was not provided.

---

# Building the Project

All commands should be run from the root project directory:

```text
Soft-Engineering-Assignment-1/
```

## Build the C Program

Run:

```bash
make
```

This compiles the C source files.

The following source files are compiled into a static library:

```text
src/course.c
src/faculty.c
src/parser.c
src/scheduler.c
```

The static library is created at:

```text
lib/libschedule.a
```

The main program is compiled and linked against the static library to create:

```text
bin/scheduler
```

---

# Running the Original Scheduler

## Windows

```bash
make run PYTHON=py
```

## macOS or Linux

```bash
make run PYTHON=python3
```

This:

1. Builds the C program if necessary.
2. Runs the scheduler.
3. Creates the text and CSV output files.
4. Runs `csv_to_excel.py`.
5. Creates the Excel workbook.

The generated output is stored in:

```text
output/
```

---

# Generating the Prolog Knowledge Base

## Windows

```bash
make knowledge PYTHON=py
```

## macOS or Linux

```bash
make knowledge PYTHON=python3
```

This command first runs the original scheduler and then executes:

```text
scripts/build_knowledge_base.py
```

The knowledge-base builder combines information from:

```text
output/class_schedule.csv
output/office_hours.csv
data/faculty_info.txt
data/syllabi/appolo_tankeh_syllabus_facts.txt
```

and generates:

```text
prolog/generated_facts.pl
```

---

# Running the Automatic Knowledge Demo

The automatic demo tests the Python, PySwip, and Prolog connection without requiring interactive input.

## Windows

```bash
make demo PYTHON=py
```

## macOS or Linux

```bash
make demo PYTHON=python3
```

The demo automatically asks several questions and prints the answers returned from the Prolog knowledge base.

This command is also useful for automated testing because no keyboard input is required.

---

# Running the Interactive Question System

## Windows

```bash
make ask PYTHON=py
```

## macOS or Linux

```bash
make ask PYTHON=python3
```

The program displays:

```text
======================================
 Faculty Schedule Question System
======================================

Type a question.
Type 'help' for examples.
Type 'exit' to quit.

Question:
```

The user can then type normal-English questions.

---

# Supported Question Examples

Examples include:

```text
Who teaches COSC 369?
```

```text
What classes does Adedoyin, Anthony teach?
```

```text
When are Dr. Appolo Tankeh's office hours?
```

```text
Which office hours are on Wednesday?
```

```text
Where is Dr. Appolo Tankeh's office?
```

```text
What is the email for Dr. Appolo Tankeh?
```

```text
What are the prerequisites for COSC 369?
```

```text
What is the attendance policy for COSC 369?
```

```text
What is the course description for COSC 369?
```

```text
When is the midterm for COSC 369?
```

```text
When does COSC 369 meet?
```

Type:

```text
help
```

to display example questions.

Type:

```text
exit
```

to leave the interactive question system.

---

# Example COSC 369 Queries

Example:

```text
Question: Who teaches COSC 369?
```

Possible output:

```text
COSC 369 is taught by Dr. Appolo Tankeh.
```

Example:

```text
Question: When are Dr. Appolo Tankeh's office hours?
```

Possible output:

```text
Office hours for Dr. Appolo Tankeh:
- Tuesday: 10:45 AM - 1:00 PM (COSC 369)
- Thursday: 10:45 AM - 1:00 PM (COSC 369)
```

Example:

```text
Question: What are the prerequisites for COSC 369?
```

Possible output:

```text
The prerequisites listed for COSC 369 are: COSC 214, COSC 330.
```

---

# Prolog Files

## `generated_facts.pl`

This file contains automatically generated Prolog facts.

Examples of generated facts include:

```prolog
teaches('Dr. Appolo Tankeh', 'COSC 369').

syllabus_office('Dr. Appolo Tankeh', 'CS 108').

syllabus_email(
    'Dr. Appolo Tankeh',
    'atankeh@bowiestate.edu'
).

prerequisite('COSC 369', 'COSC 214').

prerequisite('COSC 369', 'COSC 330').
```

## `rules.pl`

This file contains Prolog rules that use the generated facts.

For example:

```prolog
teaches_course(Professor, Course) :-
    teaches(Professor, Course).
```

and:

```prolog
course_prerequisite(Course, Prerequisite) :-
    prerequisite(Course, Prerequisite).
```

The facts represent stored information.

The rules allow Prolog to reason over and query that information.

---

# Testing Prolog Directly

SWI-Prolog can also be used directly.

Start Prolog:

```bash
swipl
```

Load the generated facts:

```prolog
consult('prolog/generated_facts.pl').
```

Load the rules:

```prolog
consult('prolog/rules.pl').
```

Example query:

```prolog
teaches_course(Professor, 'COSC 369').
```

Example result:

```text
Professor = 'Dr. Appolo Tankeh'.
```

Example prerequisite query:

```prolog
course_prerequisite('COSC 369', Prerequisite).
```

Prolog may return more than one result.

Press:

```text
;
```

to request the next solution.

Exit SWI-Prolog with:

```prolog
halt.
```

---

# Important Makefile Tab Rule

Makefile command lines underneath a target must begin with an actual TAB character.

For example:

```makefile
knowledge: run
	$(PYTHON) $(SCRIPT_DIR)/build_knowledge_base.py \
		$(OUT_DIR)/class_schedule.csv \
		$(OUT_DIR)/office_hours.csv \
		$(FACULTY_FILE) \
		$(SYLLABUS_FACTS) \
		$(GENERATED_FACTS)
```

The line beginning with:

```makefile
$(PYTHON)
```

must have a TAB before it.

Do not replace the TAB with spaces.

The same rule applies to commands underneath targets such as:

```text
$(STATIC_LIB):
$(TARGET):
$(BUILD_DIR)/%.o:
run:
knowledge:
ask:
demo:
clean:
```

If spaces are used instead of a TAB, Make may display an error such as:

```text
*** missing separator. Stop.
```

---

# Makefile Commands

The primary Makefile commands are:

```bash
make
```

Build the C program.

```bash
make run
```

Run the scheduler and generate the standard output files.

```bash
make knowledge
```

Run the scheduler and generate the Prolog knowledge base.

```bash
make demo
```

Generate the knowledge base and run automatic example questions.

```bash
make ask
```

Generate the knowledge base and start the interactive question system.

```bash
make docs
```

Alias for `make run`.

```bash
make clean
```

Remove generated build and output files.

---

# Windows Python Override

On this Windows configuration, the `py` launcher is used to ensure that the correct Windows Python installation is used instead of the Python installation provided by MSYS2.

Therefore the recommended Windows commands are:

```bash
make run PYTHON=py
```

```bash
make knowledge PYTHON=py
```

```bash
make demo PYTHON=py
```

```bash
make ask PYTHON=py
```

The Makefile uses:

```makefile
PYTHON ?= python
```

This allows the Python command to be overridden without modifying the Makefile.

On macOS, Linux, or in the GitLab CI pipeline, use:

```bash
make run PYTHON=python3
```

```bash
make knowledge PYTHON=python3
```

```bash
make demo PYTHON=python3
```

---

# Cleaning and Rebuilding the Project

To remove generated build/output files:

```bash
make clean
```

Then rebuild:

```bash
make
```

On Windows, a complete test can be performed with:

```bash
make clean
make
make run PYTHON=py
make knowledge PYTHON=py
make demo PYTHON=py
make ask PYTHON=py
```

---

# When Syllabus Facts Are Changed

If information inside:

```text
data/syllabi/appolo_tankeh_syllabus_facts.txt
```

is changed, `generated_facts.pl` must be regenerated.

Run:

```bash
make knowledge PYTHON=py
```

on Windows.

Or:

```bash
make knowledge PYTHON=python3
```

on Linux.

Do not manually modify:

```text
prolog/generated_facts.pl
```

because it will be overwritten the next time the knowledge base is generated.

---

# Git Ignore Notes

The following generated/local items should not be committed:

```text
.venv/
build/
lib/
bin/
__pycache__/
*.pyc
prolog/generated_facts.pl
.DS_Store
```

The `.venv` directory contains local Python packages and can be recreated using `requirements.txt`.

`generated_facts.pl` can be recreated using:

```bash
make knowledge
```

---

# Continuous Integration (GitLab CI/CD)

The repository includes a GitLab pipeline defined in `.gitlab-ci.yml`. It
runs automatically on every push to GitLab and checks that the whole
C → Python → Prolog chain still works on a clean machine.

All jobs use the `python:3.12-bookworm` Docker image, which already
includes `gcc` and `make`.

| Stage       | Job         | What it does                                                                                                 | Artifacts                       |
|-------------|-------------|--------------------------------------------------------------------------------------------------------------|---------------------------------|
| `build`     | `build`     | `make all` with `-Werror` (any compiler warning fails the build); checks `lib/libschedule.a` and `bin/scheduler` exist | `lib/`, `bin/`                  |
| `schedule`  | `schedule`  | Installs `openpyxl`, runs `make run`, checks all four output files are non-empty, prints the report          | `output/` (incl. the `.xlsx`)   |
| `knowledge` | `knowledge` | Installs SWI-Prolog and `requirements.txt`, checks PySwip, runs `make demo`                                  | `prolog/generated_facts.pl`     |

`make demo` is used instead of `make ask` because the interactive question
system waits for keyboard input, which a CI job cannot provide.

## Pushing to GitLab and GitHub

The project is kept on two remotes:

```bash
git remote -v
# origin  https://github.com/Ari3106/Soft-Engineering-Assignment-1.git
# gitlab  https://gitlab.com/cs369-group/soft-engineering-assignment-2.git
```

Push to both after committing:

```bash
git push origin Mock    # GitHub (no pipeline runs here)
git push gitlab Mock    # GitLab (starts the pipeline)
```

To add the GitLab remote on a new clone:

```bash
git remote add gitlab https://gitlab.com/cs369-group/soft-engineering-assignment-2.git
```

Neither site accepts your account password for Git over HTTPS. Use a
personal access token as the password instead:

- **GitLab:** avatar → Edit profile → Access tokens → Generate token →
  Legacy token, with the `read_repository` and `write_repository` scopes.
- **GitHub:** Settings → Developer settings → Personal access tokens →
  Tokens (classic), with the `repo` scope. Or run `gh auth login`.

## Viewing results

1. In the GitLab project, open **Build → Pipelines**.
2. Click a pipeline to see its stages, then click a job to read its log.
3. To download the generated files, click the download button on the
   pipeline row and choose **`schedule:archive`**. This gives you the
   `output/` folder with `faculty_schedule.xlsx`. They are also listed
   under **Build → Artifacts**.

To re-run without pushing: **Build → Pipelines → New pipeline**, pick the
branch, and click **New pipeline**.

## Running the same checks locally

```bash
make clean
make all CFLAGS="-Wall -Wextra -Werror -std=c11 -Iinclude -g"
make run PYTHON=python3
make demo PYTHON=python3     # requires SWI-Prolog
```

---

# Troubleshooting

## GitLab push fails with `HTTP Basic: Access denied`

GitLab rejected the password. Create a personal access token (see
[Pushing to GitLab and GitHub](#pushing-to-gitlab-and-github)), clear the
saved credential, and push again, pasting the token as the password:

```bash
printf "protocol=https\nhost=gitlab.com\n\n" | git credential-osxkeychain erase   # macOS
git push -u gitlab Mock
```

---

## Pipeline fails immediately with no jobs

If **Build → Jobs** shows 0 jobs and the Pipelines page says *"Before you
can run pipelines, we need to verify your account"*, it is not a problem
with `.gitlab-ci.yml`. Free gitlab.com accounts must verify their identity
before they can use GitLab's shared runners. Click **Verify my account**,
then start a **New pipeline**.

---

## `No module named pip`

If the MSYS2 Python installation is being used accidentally, use the Windows Python launcher:

```powershell
py -3.12 --version
```

Create and activate the virtual environment:

```powershell
py -3.12 -m venv .venv
.\.venv\Scripts\Activate.ps1
```

Then install the requirements:

```powershell
python -m pip install -r requirements.txt
```

---

## `No module named pyswip`

Install the dependencies:

```powershell
python -m pip install -r requirements.txt
```

Verify PySwip:

```powershell
python -c "from pyswip import Prolog; print('PySwip imported successfully')"
```

---

## `swipl` Is Not Recognized

Verify that SWI-Prolog is installed and that `swipl` was added to the current user's PATH.

After installation, completely close and reopen VS Code.

Then run:

```powershell
swipl --version
```

---

## Prolog Cannot Find a Predicate

If an error similar to:

```text
existence_error(procedure, ...)
```

appears, regenerate the knowledge base:

```bash
make knowledge PYTHON=py
```

Then verify that the necessary facts are present in:

```text
prolog/generated_facts.pl
```

---

## Syllabus Information Does Not Appear

Make sure the syllabus facts filename is exactly:

```text
appolo_tankeh_syllabus_facts.txt
```

The instructor's first name is spelled:

```text
Appolo
```

not:

```text
Apollo
```

After correcting the file or its contents, regenerate the knowledge base:

```bash
make knowledge PYTHON=py
```

---

## `missing separator` Make Error

If Make reports:

```text
*** missing separator. Stop.
```

check the Makefile command lines.

Commands underneath Makefile targets must begin with an actual TAB character rather than spaces.

---

# Summary

The system combines C, Python, and Prolog.

```text
C
|
|-- reads the original input files
|-- generates teaching schedules
|-- generates office-hour schedules
|-- produces CSV and text output
|
v
Python
|
|-- creates the Excel workbook
|-- reads generated schedule data
|-- reads available syllabus facts
|-- generates Prolog facts
|
v
SWI-Prolog
|
|-- stores facts
|-- applies rules
|-- answers queries
|
v
PySwip
|
|-- connects Python to SWI-Prolog
|
v
ask_questions.py
|
|-- accepts normal-English questions
|-- displays the answers to the user
```

This allows the original scheduler to continue operating as a C program while adding a Prolog-based knowledge and question system.

The GitLab CI/CD pipeline runs this entire chain on every push, so a change that breaks the C build, the Excel generation, or the Prolog knowledge base is caught right away.
