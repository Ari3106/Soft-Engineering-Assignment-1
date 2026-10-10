#!/usr/bin/env python3

import argparse
import re
from pathlib import Path

from pyswip import Prolog


# ============================================================
# HELPER FUNCTIONS
# ============================================================

def text(value):
    """
    Convert values returned by PySwip into normal Python text.
    """

    if isinstance(value, bytes):
        return value.decode("utf-8")

    return str(value)


def prolog_atom(value):
    """
    Safely place user text inside a quoted Prolog atom.

    Example:

    COSC 369

    becomes:

    'COSC 369'
    """

    value = str(value).strip()

    value = value.replace("\\", "\\\\")
    value = value.replace("'", "\\'")

    return f"'{value}'"


def clean_name(name):
    """
    Clean professor names typed in normal English.

    Example:

    Dr. Appolo Tankeh's

    becomes:

    Dr. Appolo Tankeh
    """

    name = name.strip()

    name = re.sub(
        r"[?.!]+$",
        "",
        name
    )

    name = re.sub(
        r"(?:'s|’s)$",
        "",
        name,
        flags=re.IGNORECASE
    )

    return name.strip()


def normalize_question(question):
    """
    Remove extra spaces and ending punctuation.
    """

    question = question.strip()

    question = re.sub(
        r"\s+",
        " ",
        question
    )

    question = question.rstrip(
        "?.!"
    )

    return question


# ============================================================
# WHO TEACHES A COURSE?
# ============================================================

def who_teaches(
    prolog,
    course
):

    query = (
        "teaches_course("
        "Professor, "
        f"{prolog_atom(course)}"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find an instructor "
            f"for {course}."
        )


    professors = sorted(
        {
            text(result["Professor"])
            for result in results
        }
    )


    return (
        f"{course} is taught by "
        +
        ", ".join(professors)
        +
        "."
    )


# ============================================================
# WHAT COURSES DOES A PROFESSOR TEACH?
# ============================================================

def professor_courses(
    prolog,
    professor
):

    query = (
        "professor_course("
        f"{prolog_atom(professor)}, "
        "Course"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find courses taught "
            f"by {professor}."
        )


    courses = sorted(
        {
            text(result["Course"])
            for result in results
        }
    )


    return (
        f"{professor} teaches: "
        +
        ", ".join(courses)
        +
        "."
    )


# ============================================================
# PROFESSOR OFFICE HOURS
# ============================================================

def professor_office_hours(
    prolog,
    professor
):

    query = (
        "professor_office_hours("
        f"{prolog_atom(professor)}, "
        "Day, Start, End, Course"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find office hours "
            f"for {professor}."
        )


    lines = [
        f"Office hours for {professor}:"
    ]


    for result in results:

        day = text(
            result["Day"]
        ).capitalize()

        start = text(
            result["Start"]
        )

        end = text(
            result["End"]
        )

        course = text(
            result["Course"]
        )


        lines.append(
            f"- {day}: "
            f"{start} - {end} "
            f"({course})"
        )


    return "\n".join(lines)


# ============================================================
# OFFICE HOURS ON A SPECIFIC DAY
# ============================================================

def office_hours_on_day(
    prolog,
    day
):

    day = day.lower()


    query = (
        "office_hours_on_day("
        f"{day}, "
        "Professor, Start, End, Course"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find office hours "
            f"on {day.capitalize()}."
        )


    lines = [
        f"Office hours on {day.capitalize()}:"
    ]


    for result in results:

        professor = text(
            result["Professor"]
        )

        start = text(
            result["Start"]
        )

        end = text(
            result["End"]
        )

        course = text(
            result["Course"]
        )


        lines.append(
            f"- {professor}: "
            f"{start} - {end} "
            f"({course})"
        )


    return "\n".join(lines)


# ============================================================
# PROFESSOR DEPARTMENT
# ============================================================

def professor_department(
    prolog,
    professor
):

    query = (
        "department("
        f"{prolog_atom(professor)}, "
        "Department"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find a department "
            f"for {professor}."
        )


    department = text(
        results[0]["Department"]
    )


    return (
        f"{professor} is in the "
        f"{department} department."
    )


# ============================================================
# PROFESSOR OFFICE
# ============================================================

def professor_office(
    prolog,
    professor
):

    # First check the syllabus facts.
    query = (
        "syllabus_office("
        f"{prolog_atom(professor)}, "
        "Office"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if results:

        office = text(
            results[0]["Office"]
        )

        return (
            f"{professor}'s office is "
            f"{office}."
        )


    # If not found in a syllabus,
    # check the original faculty information.
    query = (
        "office_room("
        f"{prolog_atom(professor)}, "
        "Office"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if results:

        office = text(
            results[0]["Office"]
        )

        return (
            f"{professor}'s office is "
            f"{office}."
        )


    return (
        f"I could not find an office "
        f"for {professor}."
    )


# ============================================================
# PROFESSOR EMAIL
# ============================================================

def professor_email(
    prolog,
    professor
):

    query = (
        "syllabus_email("
        f"{prolog_atom(professor)}, "
        "Email"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find an email "
            f"for {professor} in the "
            f"available syllabus information."
        )


    email = text(
        results[0]["Email"]
    )


    return (
        f"{professor}'s email is "
        f"{email}."
    )


# ============================================================
# COURSE PREREQUISITES
# ============================================================

def course_prerequisites(
    prolog,
    course
):

    query = (
        "course_prerequisite("
        f"{prolog_atom(course)}, "
        "Prerequisite"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find prerequisite "
            f"information for {course}."
        )


    prerequisites = sorted(
        {
            text(result["Prerequisite"])
            for result in results
        }
    )


    return (
        f"The prerequisites listed for "
        f"{course} are: "
        +
        ", ".join(prerequisites)
        +
        "."
    )


# ============================================================
# COURSE DESCRIPTION
# ============================================================

def syllabus_description(
    prolog,
    course
):

    query = (
        "syllabus_description("
        f"{prolog_atom(course)}, "
        "Description"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find a syllabus "
            f"description for {course}."
        )


    description = text(
        results[0]["Description"]
    )


    return description


# ============================================================
# COURSE POLICY
# ============================================================

def syllabus_policy(
    prolog,
    course,
    policy
):

    policy = policy.lower().strip()


    query = (
        "syllabus_policy("
        f"{prolog_atom(course)}, "
        f"{prolog_atom(policy)}, "
        "PolicyText"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find the "
            f"{policy} policy for {course}."
        )


    policy_text = text(
        results[0]["PolicyText"]
    )


    return policy_text


# ============================================================
# COURSE EVENTS
# ============================================================

def syllabus_event(
    prolog,
    course,
    event
):

    event = event.lower().strip()


    query = (
        "syllabus_event("
        f"{prolog_atom(course)}, "
        f"{prolog_atom(event)}, "
        "Date"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find the date for "
            f"{event} in {course}."
        )


    event_date = text(
        results[0]["Date"]
    )


    return (
        f"The {event} for {course} "
        f"is listed as {event_date}."
    )


# ============================================================
# COURSE MEETING TIMES
# ============================================================

def course_meetings(
    prolog,
    course
):

    query = (
        "course_meeting("
        f"{prolog_atom(course)}, "
        "Day, Start, End"
        ")"
    )

    results = list(
        prolog.query(query)
    )


    if not results:

        return (
            f"I could not find meeting times "
            f"for {course}."
        )


    lines = [
        f"{course} meets:"
    ]


    for result in results:

        day = text(
            result["Day"]
        ).capitalize()

        start = text(
            result["Start"]
        )

        end = text(
            result["End"]
        )


        lines.append(
            f"- {day}: {start} - {end}"
        )


    return "\n".join(lines)


# ============================================================
# QUESTION INTERPRETER
# ============================================================

def answer_question(
    prolog,
    question
):

    question = normalize_question(
        question
    )


    # --------------------------------------------------------
    # Who teaches COSC 369?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"who teaches\s+(.+)",
        question,
        re.IGNORECASE
    )

    if match:

        course = match.group(1).strip()

        return who_teaches(
            prolog,
            course
        )


    # --------------------------------------------------------
    # What classes does NAME teach?
    # What courses does NAME teach?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"what (?:classes|courses) does\s+(.+?)\s+teach",
        question,
        re.IGNORECASE
    )

    if match:

        professor = clean_name(
            match.group(1)
        )

        return professor_courses(
            prolog,
            professor
        )


    # --------------------------------------------------------
    # When are NAME's office hours?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"when are\s+(.+?)\s+office hours",
        question,
        re.IGNORECASE
    )

    if match:

        professor = clean_name(
            match.group(1)
        )

        return professor_office_hours(
            prolog,
            professor
        )


    # --------------------------------------------------------
    # Which office hours are on Wednesday?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"which office hours are on\s+"
        r"(monday|tuesday|wednesday|thursday|friday)",
        question,
        re.IGNORECASE
    )

    if match:

        day = match.group(1)

        return office_hours_on_day(
            prolog,
            day
        )


    # --------------------------------------------------------
    # Where is NAME's office?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"where is\s+(.+?)\s+office",
        question,
        re.IGNORECASE
    )

    if match:

        professor = clean_name(
            match.group(1)
        )

        return professor_office(
            prolog,
            professor
        )


    # --------------------------------------------------------
    # What department is NAME in?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"what department is\s+(.+?)\s+in",
        question,
        re.IGNORECASE
    )

    if match:

        professor = clean_name(
            match.group(1)
        )

        return professor_department(
            prolog,
            professor
        )


    # --------------------------------------------------------
    # What is the email for NAME?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"what is the email for\s+(.+)",
        question,
        re.IGNORECASE
    )

    if match:

        professor = clean_name(
            match.group(1)
        )

        return professor_email(
            prolog,
            professor
        )


    # --------------------------------------------------------
    # What are the prerequisites for COSC 369?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"what are the prerequisites for\s+(.+)",
        question,
        re.IGNORECASE
    )

    if match:

        course = match.group(1).strip()

        return course_prerequisites(
            prolog,
            course
        )


    # --------------------------------------------------------
    # What is the course description for COSC 369?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"what is the course description for\s+(.+)",
        question,
        re.IGNORECASE
    )

    if match:

        course = match.group(1).strip()

        return syllabus_description(
            prolog,
            course
        )


    # --------------------------------------------------------
    # What is the attendance policy for COSC 369?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"what is the\s+(.+?)\s+policy for\s+(.+)",
        question,
        re.IGNORECASE
    )

    if match:

        policy = match.group(1).strip()
        course = match.group(2).strip()

        return syllabus_policy(
            prolog,
            course,
            policy
        )


    # --------------------------------------------------------
    # When is the midterm for COSC 369?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"when is the\s+(.+?)\s+for\s+(.+)",
        question,
        re.IGNORECASE
    )

    if match:

        event = match.group(1).strip()
        course = match.group(2).strip()

        return syllabus_event(
            prolog,
            course,
            event
        )


    # --------------------------------------------------------
    # When does COSC 369 meet?
    # --------------------------------------------------------

    match = re.fullmatch(
        r"when does\s+(.+?)\s+meet",
        question,
        re.IGNORECASE
    )

    if match:

        course = match.group(1).strip()

        return course_meetings(
            prolog,
            course
        )


    return (
        "I do not recognize that question yet. "
        "Type help to see example questions."
    )


# ============================================================
# HELP MENU
# ============================================================

def show_help():

    print()
    print("Example questions:")
    print()

    print("  Who teaches COSC 369?")

    print(
        "  What classes does "
        "Adedoyin, Anthony teach?"
    )

    print(
        "  When are Dr. Appolo Tankeh's "
        "office hours?"
    )

    print(
        "  Which office hours are on Wednesday?"
    )

    print(
        "  Where is Dr. Appolo Tankeh's office?"
    )

    print(
        "  What is the email for "
        "Dr. Appolo Tankeh?"
    )

    print(
        "  What are the prerequisites "
        "for COSC 369?"
    )

    print(
        "  What is the attendance policy "
        "for COSC 369?"
    )

    print(
        "  What is the course description "
        "for COSC 369?"
    )

    print(
        "  When is the midterm for COSC 369?"
    )

    print(
        "  When does COSC 369 meet?"
    )

    print()


# ============================================================
# DEMO
# ============================================================

def run_demo(
    prolog
):

    questions = [

        "Who teaches COSC 369?",

        (
            "When are Dr. Appolo Tankeh's "
            "office hours?"
        ),

        (
            "Which office hours are "
            "on Wednesday?"
        ),

        (
            "What are the prerequisites "
            "for COSC 369?"
        ),
    ]


    print()
    print("======================================")
    print(" Faculty Schedule Knowledge Demo")
    print("======================================")


    for question in questions:

        print()

        print(
            f"Question: {question}"
        )

        print(
            "Answer:"
        )

        print(
            answer_question(
                prolog,
                question
            )
        )


    print()


# ============================================================
# MAIN PROGRAM
# ============================================================

def main():

    parser = argparse.ArgumentParser(
        description=(
            "Ask questions about faculty, "
            "courses, office hours, and "
            "syllabus information."
        )
    )


    parser.add_argument(
        "--facts",
        default="prolog/generated_facts.pl",
        help="Path to generated Prolog facts."
    )


    parser.add_argument(
        "--rules",
        default="prolog/rules.pl",
        help="Path to Prolog rules."
    )


    parser.add_argument(
        "--demo",
        action="store_true",
        help="Run automatic demonstration questions."
    )


    args = parser.parse_args()


    facts_file = Path(
        args.facts
    )


    rules_file = Path(
        args.rules
    )


    if not facts_file.exists():

        print(
            f"Error: facts file not found: "
            f"{facts_file}"
        )

        print(
            "Run build_knowledge_base.py first."
        )

        return 1


    if not rules_file.exists():

        print(
            f"Error: rules file not found: "
            f"{rules_file}"
        )

        return 1


    prolog = Prolog()


    # Load generated facts.
    prolog.consult(
        str(
            facts_file.resolve()
        )
    )


    # Load rules.
    prolog.consult(
        str(
            rules_file.resolve()
        )
    )


    if args.demo:

        run_demo(
            prolog
        )

        return 0


    print()
    print("======================================")
    print(" Faculty Schedule Question System")
    print("======================================")
    print()
    print("Type a question.")
    print("Type 'help' for examples.")
    print("Type 'exit' to quit.")
    print()


    while True:

        question = input(
            "Question: "
        ).strip()


        if not question:

            continue


        if question.lower() in {
            "exit",
            "quit"
        }:

            print(
                "Goodbye."
            )

            break


        if question.lower() == "help":

            show_help()

            continue


        print()

        print(
            answer_question(
                prolog,
                question
            )
        )

        print()


    return 0


if __name__ == "__main__":

    raise SystemExit(
        main()
    )