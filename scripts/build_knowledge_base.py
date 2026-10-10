#!/usr/bin/env python3

import csv
import sys
from pathlib import Path


# ------------------------------------------------------------
# Convert the one-letter day codes used by the C scheduler
# into readable Prolog atoms.
#
# R is used for Thursday because T is already Tuesday.
# ------------------------------------------------------------

DAY_LETTERS = {
    "M": "monday",
    "T": "tuesday",
    "W": "wednesday",
    "R": "thursday",
    "F": "friday",
}


# ------------------------------------------------------------
# Convert text into a safe quoted Prolog atom.
#
# Example:
#
# Adedoyin, Anthony
#
# becomes:
#
# 'Adedoyin, Anthony'
# ------------------------------------------------------------

def quote_atom(value):

    text = str(value).strip()

    # Escape backslashes.
    text = text.replace(
        "\\",
        "\\\\"
    )

    # Escape single quotation marks.
    text = text.replace(
        "'",
        "\\'"
    )

    return f"'{text}'"


# ------------------------------------------------------------
# Add a normal Prolog fact.
#
# Example:
#
# add_fact(
#     facts,
#     "teaches",
#     "Adedoyin, Anthony",
#     "CTEC350.170"
# )
#
# generates:
#
# teaches(
#     'Adedoyin, Anthony',
#     'CTEC350.170'
# ).
# ------------------------------------------------------------

def add_fact(
    facts,
    predicate,
    *arguments
):

    values = []

    for argument in arguments:

        if isinstance(argument, int):

            values.append(
                str(argument)
            )

        else:

            values.append(
                quote_atom(argument)
            )


    fact = (
        f"{predicate}("
        +
        ", ".join(values)
        +
        ")."
    )


    facts.add(fact)


# ------------------------------------------------------------
# Add a fact containing a weekday.
#
# The weekday should NOT be placed in quotation marks.
#
# Example:
#
# class_meets(
#     'COSC 369',
#     tuesday,
#     '09:30 AM',
#     '10:45 AM'
# ).
# ------------------------------------------------------------

def add_day_fact(
    facts,
    predicate,
    before_day,
    day,
    after_day
):

    values = []


    for value in before_day:

        values.append(
            quote_atom(value)
        )


    # The weekday is a Prolog atom.
    values.append(day)


    for value in after_day:

        values.append(
            quote_atom(value)
        )


    fact = (
        f"{predicate}("
        +
        ", ".join(values)
        +
        ")."
    )


    facts.add(fact)


# ============================================================
# CLASS SCHEDULE
# ============================================================

def read_class_schedule(
    filename,
    facts
):

    print(
        f"Reading class schedule: {filename}"
    )


    with open(
        filename,
        newline="",
        encoding="utf-8"
    ) as file:

        reader = csv.DictReader(file)


        for row in reader:

            instructor = (
                row["Instructor"]
                .strip()
            )

            course = (
                row["Code"]
                .strip()
            )

            title = (
                row["Title"]
                .strip()
            )

            days = (
                row["Days"]
                .strip()
                .upper()
            )

            start = (
                row["Start"]
                .strip()
            )

            end = (
                row["End"]
                .strip()
            )

            room = (
                row["Room"]
                .strip()
            )


            # ----------------------------
            # Basic course fact
            # ----------------------------

            add_fact(
                facts,
                "course",
                course
            )


            # ----------------------------
            # Course title
            # ----------------------------

            add_fact(
                facts,
                "course_title",
                course,
                title
            )


            # ----------------------------
            # Professor teaches course
            # ----------------------------

            add_fact(
                facts,
                "teaches",
                instructor,
                course
            )


            # ----------------------------
            # Class room
            # ----------------------------

            add_fact(
                facts,
                "class_room",
                course,
                room
            )


            # ----------------------------
            # Meeting days
            # ----------------------------

            for letter in days:

                day = DAY_LETTERS.get(
                    letter
                )


                if day is None:

                    continue


                add_day_fact(
                    facts,
                    "class_meets",
                    [course],
                    day,
                    [
                        start,
                        end
                    ]
                )


# ============================================================
# OFFICE HOURS
# ============================================================

def read_office_hours(
    filename,
    facts
):

    print(
        f"Reading office hours: {filename}"
    )


    with open(
        filename,
        newline="",
        encoding="utf-8"
    ) as file:

        reader = csv.DictReader(file)


        for row in reader:

            instructor = (
                row["Instructor"]
                .strip()
            )

            day = (
                row["Day"]
                .strip()
                .lower()
            )

            start = (
                row["Start"]
                .strip()
            )

            end = (
                row["End"]
                .strip()
            )

            related_course = (
                row["RelatedCourse"]
                .strip()
            )


            if not related_course:

                related_course = "General"


            add_day_fact(
                facts,
                "office_hour",
                [instructor],
                day,
                [
                    start,
                    end,
                    related_course
                ]
            )


# ============================================================
# FACULTY INFORMATION
# ============================================================

def read_faculty_information(
    filename,
    facts
):

    print(
        f"Reading faculty information: {filename}"
    )


    with open(
        filename,
        encoding="utf-8"
    ) as file:

        for line_number, raw_line in enumerate(
            file,
            start=1
        ):

            line = raw_line.strip()


            # Ignore comments and blank lines.
            if (
                not line
                or
                line.startswith("#")
            ):

                continue


            fields = [
                field.strip()
                for field
                in line.split("|")
            ]


            if len(fields) != 5:

                print(
                    f"Warning: faculty line "
                    f"{line_number} has "
                    f"{len(fields)} fields instead of 5."
                )

                continue


            (
                name,
                department,
                weekly_hours,
                office_room,
                preferred_days
            ) = fields


            try:

                weekly_hours = int(
                    weekly_hours
                )

            except ValueError:

                print(
                    f"Warning: invalid office-hour "
                    f"requirement for {name}."
                )

                continue


            # ----------------------------
            # Faculty member
            # ----------------------------

            add_fact(
                facts,
                "faculty",
                name
            )


            # ----------------------------
            # Department
            # ----------------------------

            add_fact(
                facts,
                "department",
                name,
                department
            )


            # ----------------------------
            # Required office hours
            # ----------------------------

            add_fact(
                facts,
                "required_office_hours",
                name,
                weekly_hours
            )


            # ----------------------------
            # Office location
            # ----------------------------

            add_fact(
                facts,
                "office_room",
                name,
                office_room
            )


            # ----------------------------
            # Preferred days
            # ----------------------------

            for letter in preferred_days.upper():

                day = DAY_LETTERS.get(
                    letter
                )


                if day is None:

                    continue


                add_day_fact(
                    facts,
                    "preferred_day",
                    [name],
                    day,
                    []
                )


# ============================================================
# SYLLABUS FACTS
# ============================================================

def read_syllabus_information(
    filename,
    facts
):

    syllabus_file = Path(
        filename
    )


    print(
        f"Reading syllabus facts: {filename}"
    )


    if not syllabus_file.exists():

        print(
            "Warning: syllabus facts file "
            "does not exist."
        )

        return


    with syllabus_file.open(
        encoding="utf-8"
    ) as file:

        for line_number, raw_line in enumerate(
            file,
            start=1
        ):

            line = raw_line.strip()


            # Ignore comments and blanks.
            if (
                not line
                or
                line.startswith("#")
            ):

                continue


            fact_type = (
                line
                .split("|", 1)[0]
                .strip()
                .upper()
            )


            try:

                # ============================================
                # COURSE
                # ============================================

                if fact_type == "COURSE":

                    _, course, title = [
                        item.strip()
                        for item
                        in line.split("|", 2)
                    ]


                    add_fact(
                        facts,
                        "syllabus_course",
                        course,
                        title
                    )


                    add_fact(
                        facts,
                        "course",
                        course
                    )


                    add_fact(
                        facts,
                        "course_title",
                        course,
                        title
                    )


                # ============================================
                # INSTRUCTOR
                # ============================================

                elif fact_type == "INSTRUCTOR":

                    _, professor, course = [
                        item.strip()
                        for item
                        in line.split("|", 2)
                    ]


                    add_fact(
                        facts,
                        "syllabus_instructor",
                        course,
                        professor
                    )


                    add_fact(
                        facts,
                        "teaches",
                        professor,
                        course
                    )


                # ============================================
                # EMAIL
                # ============================================

                elif fact_type == "EMAIL":

                    _, professor, email = [
                        item.strip()
                        for item
                        in line.split("|", 2)
                    ]


                    add_fact(
                        facts,
                        "syllabus_email",
                        professor,
                        email
                    )


                # ============================================
                # OFFICE
                # ============================================

                elif fact_type == "OFFICE":

                    _, professor, office = [
                        item.strip()
                        for item
                        in line.split("|", 2)
                    ]


                    add_fact(
                        facts,
                        "syllabus_office",
                        professor,
                        office
                    )


                # ============================================
                # CLASS TIME
                # ============================================

                elif fact_type == "CLASS_TIME":

                    (
                        _,
                        course,
                        days,
                        start,
                        end
                    ) = [
                        item.strip()
                        for item
                        in line.split("|", 4)
                    ]


                    for letter in days.upper():

                        day = DAY_LETTERS.get(
                            letter
                        )


                        if day is None:

                            continue


                        add_day_fact(
                            facts,
                            "class_meets",
                            [course],
                            day,
                            [
                                start,
                                end
                            ]
                        )


                # ============================================
                # OFFICE HOURS
                # ============================================

                elif fact_type == "OFFICE_HOURS":

                    (
                        _,
                        professor,
                        days,
                        start,
                        end
                    ) = [
                        item.strip()
                        for item
                        in line.split("|", 4)
                    ]


                    for letter in days.upper():

                        day = DAY_LETTERS.get(
                            letter
                        )


                        if day is None:

                            continue


                        add_day_fact(
                            facts,
                            "office_hour",
                            [professor],
                            day,
                            [
                                start,
                                end,
                                "COSC 369"
                            ]
                        )


                # ============================================
                # PREREQUISITE
                # ============================================

                elif fact_type == "PREREQUISITE":

                    (
                        _,
                        course,
                        prerequisite
                    ) = [
                        item.strip()
                        for item
                        in line.split("|", 2)
                    ]


                    add_fact(
                        facts,
                        "prerequisite",
                        course,
                        prerequisite
                    )


                # ============================================
                # DESCRIPTION
                # ============================================

                elif fact_type == "DESCRIPTION":

                    (
                        _,
                        course,
                        description
                    ) = [
                        item.strip()
                        for item
                        in line.split("|", 2)
                    ]


                    add_fact(
                        facts,
                        "syllabus_description",
                        course,
                        description
                    )


                # ============================================
                # POLICY
                # ============================================

                elif fact_type == "POLICY":

                    (
                        _,
                        course,
                        policy,
                        policy_text
                    ) = [
                        item.strip()
                        for item
                        in line.split("|", 3)
                    ]


                    add_fact(
                        facts,
                        "syllabus_policy",
                        course,
                        policy.lower(),
                        policy_text
                    )


                # ============================================
                # EVENT
                # ============================================

                elif fact_type == "EVENT":

                    (
                        _,
                        course,
                        event,
                        event_date
                    ) = [
                        item.strip()
                        for item
                        in line.split("|", 3)
                    ]


                    add_fact(
                        facts,
                        "syllabus_event",
                        course,
                        event.lower(),
                        event_date
                    )


                else:

                    print(
                        f"Warning: unknown syllabus "
                        f"fact type '{fact_type}' "
                        f"on line {line_number}."
                    )


            except ValueError:

                print(
                    f"Warning: malformed syllabus "
                    f"fact on line {line_number}:"
                )

                print(
                    line
                )


# ============================================================
# WRITE PROLOG FILE
# ============================================================

def write_knowledge_base(
    filename,
    facts
):

    output = Path(
        filename
    )


    output.parent.mkdir(
        parents=True,
        exist_ok=True
    )


    with output.open(
        "w",
        encoding="utf-8"
    ) as file:

        file.write(
            "% ============================================================\n"
        )

        file.write(
            "% GENERATED PROLOG KNOWLEDGE BASE\n"
        )

        file.write(
            "% Generated automatically by build_knowledge_base.py\n"
        )

        file.write(
            "% Do not edit this file manually.\n"
        )

        file.write(
            "% ============================================================\n\n"
        )


        for fact in sorted(facts):

            file.write(
                fact
            )

            file.write(
                "\n"
            )


    print()

    print(
        "Knowledge base successfully created:"
    )

    print(
        output
    )

    print()

    print(
        f"Total unique facts: {len(facts)}"
    )


# ============================================================
# MAIN
# ============================================================

def main():

    if len(sys.argv) != 6:

        print()

        print(
            "Usage:"
        )

        print(
            "python build_knowledge_base.py "
            "<class_schedule.csv> "
            "<office_hours.csv> "
            "<faculty_info.txt> "
            "<syllabus_facts.txt> "
            "<output.pl>"
        )

        print()

        return 1


    class_file = sys.argv[1]

    office_file = sys.argv[2]

    faculty_file = sys.argv[3]

    syllabus_file = sys.argv[4]

    output_file = sys.argv[5]


    facts = set()


    read_class_schedule(
        class_file,
        facts
    )


    read_office_hours(
        office_file,
        facts
    )


    read_faculty_information(
        faculty_file,
        facts
    )


    read_syllabus_information(
        syllabus_file,
        facts
    )


    write_knowledge_base(
        output_file,
        facts
    )


    return 0


if __name__ == "__main__":

    raise SystemExit(
        main()
    )