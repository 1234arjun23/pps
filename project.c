/*
  Advanced Student Result Analyzer (Windows-friendly C)
  ----------------------------------------------------
  Features:
  - Add / Delete / Modify / Search students
  - Calculate total, percentage, grade
  - Sort by percentage (descending)
  - Per-student ASCII bar graph (per subject)
  - Class analysis: subject averages and grade distribution (ASCII)
  - Save / Load to "students.dat" (binary)
  - Menu driven

  Compile (MinGW / GCC):
    gcc -o student_analyzer student_analyzer.c

  Run:
    student_analyzer.exe   (or ./student_analyzer)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 200
#define MAX_NAME 60
#define SUBJECTS 5
#define DATAFILE "students.dat"
#define BAR_WIDTH 40

typedef struct {
    int roll;
    char name[MAX_NAME];
    int marks[SUBJECTS]; // marks for SUBJECTS subjects
    int total;
    float percentage;
    char grade; // 'A','B','C','D','F'
} Student;

/* ---------- Global storage ---------- */
Student students[MAX_STUDENTS];
int student_count = 0;

/* ---------- Helper input functions ---------- */
void read_line(char *buf, int size) {
    if (fgets(buf, size, stdin) != NULL) {
        size_t ln = strlen(buf);
        if (ln > 0 && buf[ln-1] == '\n') buf[ln-1] = '\0';
    }
}

int read_int(const char *prompt) {
    char buf[64];
    int val;
    while (1) {
        printf("%s", prompt);
        read_line(buf, sizeof buf);
        if (sscanf(buf, "%d", &val) == 1) return val;
        printf("  Invalid number, try again.\n");
    }
}

/* ---------- Result calculations ---------- */
char compute_grade(float perc) {
    if (perc >= 85.0f) return 'A';
    if (perc >= 70.0f) return 'B';
    if (perc >= 55.0f) return 'C';
    if (perc >= 40.0f) return 'D';
    return 'F';
}

void calculate_result(Student *s) {
    int tot = 0;
    for (int i = 0; i < SUBJECTS; ++i) {
        if (s->marks[i] < 0) s->marks[i] = 0;
        if (s->marks[i] > 100) s->marks[i] = 100;
        tot += s->marks[i];
    }
    s->total = tot;
    s->percentage = (float)tot / (SUBJECTS);
    s->grade = compute_grade(s->percentage);
}

/* ---------- File I/O ---------- */
void save_to_file() {
    FILE *f = fopen(DATAFILE, "wb");
    if (!f) {
        printf("Error: Could not open %s for writing.\n", DATAFILE);
        return;
    }
    fwrite(&student_count, sizeof(int), 1, f);
    fwrite(students, sizeof(Student), student_count, f);
    fclose(f);
    printf("Saved %d students to %s.\n", student_count, DATAFILE);
}

void load_from_file() {
    FILE *f = fopen(DATAFILE, "rb");
    if (!f) {
        printf("No existing data file found. Starting with empty list.\n");
        student_count = 0;
        return;
    }
    fread(&student_count, sizeof(int), 1, f);
    if (student_count > MAX_STUDENTS) student_count = 0;
    fread(students, sizeof(Student), student_count, f);
    fclose(f);
    printf("Loaded %d students from %s.\n", student_count, DATAFILE);
}

/* ---------- CRUD operations ---------- */
int find_index_by_roll(int roll) {
    for (int i = 0; i < student_count; ++i)
        if (students[i].roll == roll) return i;
    return -1;
}

void add_student() {
    if (student_count >= MAX_STUDENTS) {
        printf("Student list full (max %d).\n", MAX_STUDENTS);
        return;
    }
    Student s;
    printf("\n--- Add New Student ---\n");
    s.roll = read_int("Enter roll number: ");
    if (find_index_by_roll(s.roll) != -1) {
        printf("A student with roll %d already exists.\n", s.roll);
        return;
    }
    printf("Enter name: ");
    read_line(s.name, MAX_NAME);
    for (int i = 0; i < SUBJECTS; ++i) {
        char prompt[80];
        sprintf(prompt, "Enter marks for Subject %d (0-100): ", i+1);
        s.marks[i] = read_int(prompt);
    }
    calculate_result(&s);
    students[student_count++] = s;
    printf("Student added. Percentage: %.2f Grade: %c\n\n", s.percentage, s.grade);
}

void display_student(const Student *s) {
    printf("Roll: %d\nName: %s\n", s->roll, s->name);
    for (int i = 0; i < SUBJECTS; ++i) {
        printf(" Subject %d : %3d\n", i+1, s->marks[i]);
    }
    printf(" Total : %d\n Percentage : %.2f\n Grade : %c\n", s->total, s->percentage, s->grade);
}

void display_all() {
    if (student_count == 0) {
        printf("No students to display.\n");
        return;
    }
    printf("\n--- All Students ---\n");
    printf("%-6s %-20s %-8s %-8s\n", "Roll", "Name", "Percent", "Grade");
    for (int i = 0; i < student_count; ++i) {
        printf("%-6d %-20s %-8.2f %-8c\n", students[i].roll, students[i].name, students[i].percentage, students[i].grade);
    }
    printf("\n");
}

void modify_marks() {
    int roll = read_int("Enter roll number to modify: ");
    int idx = find_index_by_roll(roll);
    if (idx == -1) {
        printf("Student with roll %d not found.\n", roll);
        return;
    }
    printf("\nModifying marks for %s (Roll %d)\n", students[idx].name, students[idx].roll);
    for (int i = 0; i < SUBJECTS; ++i) {
        char prompt[80];
        sprintf(prompt, "New marks for Subject %d (current %d, enter -1 to keep): ", i+1, students[idx].marks[i]);
        int x = read_int(prompt);
        if (x >= 0 && x <= 100) students[idx].marks[i] = x;
    }
    calculate_result(&students[idx]);
    printf("Updated. New percentage: %.2f Grade: %c\n", students[idx].percentage, students[idx].grade);
}

void delete_student() {
    int roll = read_int("Enter roll number to delete: ");
    int idx = find_index_by_roll(roll);
    if (idx == -1) {
        printf("Student with roll %d not found.\n", roll);
        return;
    }
    // shift left
    for (int i = idx; i < student_count - 1; ++i)
        students[i] = students[i+1];
    student_count--;
    printf("Deleted student with roll %d\n", roll);
}

/* ---------- Sorting ---------- */
int cmp_percentage_desc(const void *a, const void *b) {
    const Student *sa = (const Student*)a;
    const Student *sb = (const Student*)b;
    if (sa->percentage < sb->percentage) return 1;
    if (sa->percentage > sb->percentage) return -1;
    return 0;
}

void sort_by_percentage() {
    if (student_count <= 1) {
        printf("Not enough students to sort.\n");
        return;
    }
    qsort(students, student_count, sizeof(Student), cmp_percentage_desc);
    printf("Sorted by percentage (highest first).\n");
}

/* ---------- ASCII Graphs ---------- */
void draw_bar(int value, int max_value) {
    int filled = (int)((double)value / max_value * BAR_WIDTH + 0.5);
    if (filled < 0) filled = 0;
    if (filled > BAR_WIDTH) filled = BAR_WIDTH;
    for (int i = 0; i < filled; ++i) putchar('|');
    for (int i = filled; i < BAR_WIDTH; ++i) putchar(' ');
}

void graph_student() {
    int roll = read_int("Enter roll number for graph: ");
    int idx = find_index_by_roll(roll);
    if (idx == -1) {
        printf("Student with roll %d not found.\n", roll);
        return;
    }
    Student *s = &students[idx];
    printf("\n--- Performance Graph for %s (Roll %d) ---\n", s->name, s->roll);
    int max_mark = 100; // marks out of 100
    for (int i = 0; i < SUBJECTS; ++i) {
        printf("Subject %d [%3d]: ", i+1, s->marks[i]);
        draw_bar(s->marks[i], max_mark);
        printf(" %d\n", s->marks[i]);
    }
    printf("\nTotal: %d  Percentage: %.2f  Grade: %c\n\n", s->total, s->percentage, s->grade);
}

void class_analysis() {
    if (student_count == 0) {
        printf("No students for analysis.\n");
        return;
    }
    printf("\n--- Class Analysis ---\n");
    // subject averages
    double subj_sum[SUBJECTS] = {0};
    for (int i = 0; i < student_count; ++i)
        for (int j = 0; j < SUBJECTS; ++j)
            subj_sum[j] += students[i].marks[j];

    printf("Subject Averages:\n");
    int max_avg = 0;
    for (int j = 0; j < SUBJECTS; ++j) {
        double avg = subj_sum[j] / student_count;
        if ((int)(avg+0.5) > max_avg) max_avg = (int)(avg+0.5);
        printf(" Subject %d : %.2f\t", j+1, avg);
        draw_bar((int)(avg+0.5), 100);
        printf(" %.2f\n", avg);
    }

    // grade distribution
    int grades_count[5] = {0}; // A,B,C,D,F
    for (int i = 0; i < student_count; ++i) {
        switch (students[i].grade) {
            case 'A': grades_count[0]++; break;
            case 'B': grades_count[1]++; break;
            case 'C': grades_count[2]++; break;
            case 'D': grades_count[3]++; break;
            default:  grades_count[4]++; break;
        }
    }
    printf("\nGrade Distribution:\n");
    const char *labels[5] = {"A", "B", "C", "D", "F"};
    int max_grade_count = 1;
    for (int i = 0; i < 5; ++i) if (grades_count[i] > max_grade_count) max_grade_count = grades_count[i];

    for (int i = 0; i < 5; ++i) {
        printf("%2s Grade [%3d]: ", labels[i], grades_count[i]);
        // scale grade bars to BAR_WIDTH using max_grade_count
        int barlen = (int)((double)grades_count[i] / max_grade_count * BAR_WIDTH + 0.5);
        for (int b = 0; b < barlen; ++b) putchar('#');
        for (int b = barlen; b < BAR_WIDTH; ++b) putchar(' ');
        printf(" %d\n", grades_count[i]);
    }

    // class average overall
    double grand_total = 0;
    int grand_max = SUBJECTS * 100;
    for (int i = 0; i < student_count; ++i) grand_total += students[i].total;
    double class_avg = grand_total / student_count / SUBJECTS;
    printf("\nClass Average Percentage: %.2f\n", class_avg);
    printf("\n");
}

/* ---------- Utility: demo data to quickly show graphs ---------- */
void load_demo_data() {
    Student s1 = {1, "Ravi Kumar", {88, 76, 92, 85, 79}, 0, 0.0f, 'A'};
    Student s2 = {2, "Priya Sharma", {78, 81, 69, 74, 80}, 0, 0.0f, 'B'};
    Student s3 = {3, "Amit Roy", {55, 61, 49, 58, 60}, 0, 0.0f, 'C'};
    Student s4 = {4, "Sneha Gupta", {92, 95, 89, 94, 90}, 0, 0.0f, 'A'};
    Student s5 = {5, "Karan Patel", {40, 35, 50, 45, 38}, 0, 0.0f, 'D'};
    Student arr[] = {s1, s2, s3, s4, s5};
    int n = sizeof(arr)/sizeof(arr[0]);
    for (int i = 0; i < n && student_count < MAX_STUDENTS; ++i) {
        calculate_result(&arr[i]);
        students[student_count++] = arr[i];
    }
    printf("Loaded demo data (%d students).\n", n);
}

/* ---------- Menu ---------- */
void show_menu() {
    printf("====== Student Result Analyzer ======\n");
    printf("1. Add Student\n");
    printf("2. Display All Students\n");
    printf("3. Modify Marks\n");
    printf("4. Delete Student\n");
    printf("5. Sort by Percentage (High->Low)\n");
    printf("6. Show Student Graph (per-subject)\n");
    printf("7. Class Analysis (averages + grade distribution)\n");
    printf("8. Save to file\n");
    printf("9. Load from file\n");
    printf("10. Load Demo Data (quick)\n");
    printf("0. Exit\n");
    printf("Select option: ");
}

int main() {
    load_from_file(); // attempt to load existing data (if any)

    while (1) {
        show_menu();
        int choice = read_int("");
        switch (choice) {
            case 1: add_student(); break;
            case 2: display_all(); break;
            case 3: modify_marks(); break;
            case 4: delete_student(); break;
            case 5: sort_by_percentage(); break;
            case 6: graph_student(); break;
            case 7: class_analysis(); break;
            case 8: save_to_file(); break;
            case 9: load_from_file(); break;
            case 10: load_demo_data(); break;
            case 0:
                printf("Do you want to save before exit? (y/n): ");
                {
                    char ans[8];
                    read_line(ans, sizeof ans);
                    if (ans[0] == 'y' || ans[0] == 'Y') save_to_file();
                }
                printf("Exiting. Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
    return 0;
}
