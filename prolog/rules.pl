% ============================================================
% rules.pl
% Faculty Schedule Knowledge System
% ============================================================

teaches_course(Professor, Course) :-
    teaches(Professor, Course).

professor_course(Professor, Course) :-
    teaches(Professor, Course).

professor_office_hours(
    Professor,
    Day,
    Start,
    End,
    RelatedCourse
) :-
    office_hour(
        Professor,
        Day,
        Start,
        End,
        RelatedCourse
    ).

office_hours_on_day(
    Day,
    Professor,
    Start,
    End,
    RelatedCourse
) :-
    office_hour(
        Professor,
        Day,
        Start,
        End,
        RelatedCourse
    ).

course_meeting(
    Course,
    Day,
    Start,
    End
) :-
    class_meets(
        Course,
        Day,
        Start,
        End
    ).

professor_available_on(
    Professor,
    Day
) :-
    office_hour(
        Professor,
        Day,
        _,
        _,
        _
    ).

has_syllabus_information(Course) :-
    syllabus_course(
        Course,
        _
    ).

course_prerequisite(
    Course,
    Prerequisite
) :-
    prerequisite(
        Course,
        Prerequisite
    ).