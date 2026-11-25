#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define LINE_SIZE 256
#define MAX 100
#define SUBJECTS 5
#define PASS_MARK 35
typedef struct
{
    int roll_number;
    char name[100];
    float marks[SUBJECTS];
    float total;
    float percentage;
    int backlogs;
    char grade[3];
    char status[5];
}student_details;
typedef student_details students[MAX];
char subjects[SUBJECTS][30] = {"AEM I", "Physics", "Programming in C", "Problem Solving", "Environmental Studies"};
students list;
int count = 0;

int readInt(int min, int max)
{
    int value, result, c;
    while(1)
    {
        result = scanf("%d", &value);
        c = getchar();
        while(c == ' ' || c == '\t')
        {
            c = getchar();
        }
        if(c == EOF)
        {
            exit(0);
        }
        if(c != '\n')
        {
            while(c != '\n' && c != EOF)
            {
                c = getchar();
            }
            result = 0;
        }
        if(result == 1 && value >= min && value <= max)
        {
            return value;
        }
        printf("Enter a number between %d and %d: ", min, max);
    }
}

float readMark()
{
    float value;
    int result, c;
    while(1)
    {
        result = scanf("%f", &value);
        c = getchar();
        while(c == ' ' || c == '\t')
        {
            c = getchar();
        }
        if(c == EOF)
        {
            exit(0);
        }
        if(c != '\n')
        {
            while(c != '\n' && c != EOF)
            {
                c = getchar();
            }
            result = 0;
        }
        if(result == 1 && value >= 0 && value <= 100)
        {
            return value;
        }
        printf("Enter marks between 0 and 100: ");
    }
}

void readName(char name[], int size)
{
    while(1)
    {
        if(fgets(name, size, stdin) == NULL)
        {
            exit(0);
        }
        name[strcspn(name, "\n")] = '\0';
        if(strspn(name, " \t") < strlen(name))
        {
            return;
        }
        printf("Name cannot be empty, Enter again: ");
    }
}

void calculateResult(int i)
{
    int j;
    list[i].total = 0;
    list[i].backlogs = 0; 
    for(j = 0; j < SUBJECTS; j++)
    {
        list[i].total += list[i].marks[j];
        if(list[i].marks[j] < PASS_MARK) 
        {
            list[i].backlogs++;
        }
    }
    list[i].percentage = list[i].total / SUBJECTS;
    if(list[i].backlogs > 0) 
    {
        strcpy(list[i].grade, "F");
        strcpy(list[i].status, "FAIL");
    }
    else
    {
        if(list[i].percentage >= 85) strcpy(list[i].grade, "O");
        else if(list[i].percentage >= 75) strcpy(list[i].grade, "A+");
        else if(list[i].percentage >= 65) strcpy(list[i].grade, "A");
        else if(list[i].percentage >= 55) strcpy(list[i].grade, "B+");
        else if(list[i].percentage >= 45) strcpy(list[i].grade, "B");
        else if(list[i].percentage >= 40) strcpy(list[i].grade, "C+");
        else strcpy(list[i].grade, "C");
        strcpy(list[i].status, "PASS");
    }
}

void saveToFile()
{
    FILE *fp = fopen("students.txt", "w");
    int i, j;
    if(fp == NULL)
    {
        printf("Could not save file\n");
        return;
    }
    fprintf(fp, "%d\n", count);
    for(i = 0; i < count; i++)
    {
        fprintf(fp, "%d\n%s\n", list[i].roll_number, list[i].name);
        for(j = 0; j < SUBJECTS; j++)
        {
            fprintf(fp, "%.2f ", list[i].marks[j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

int readLine(FILE *fp, char line[], int size)
{
    int c;
    if(fgets(line, size, fp) == NULL)
    {
        return 0;
    }
    if((int)strlen(line) == size - 1 && line[size - 2] != '\n')
    {
        while((c = fgetc(fp)) != '\n' && c != EOF);
        line[strcspn(line, "\r\n")] = '\0';
        return -1;
    }
    line[strcspn(line, "\r\n")] = '\0';
    return 1;
}

int parseRoll(char line[])
{
    int value;
    char extra;
    if(strlen(line) > 9)
    {
        return 0;
    }
    if(sscanf(line, "%d %c", &value, &extra) != 1)
    {
        return 0;
    }
    if(value < 1 || value > 999999999)
    {
        return 0;
    }
    return value;
}

int validName(char line[])
{
    if(strlen(line) >= 100)
    {
        return 0;
    }
    return strspn(line, " \t") < strlen(line);
}

int parseMarks(char line[], float marks[])
{
    char extra;
    int j;
    if(strlen(line) > 100)
    {
        return 0;
    }
    if(sscanf(line, "%f %f %f %f %f %c", &marks[0], &marks[1], &marks[2], &marks[3], &marks[4], &extra) != 5)
    {
        return 0;
    }
    for(j = 0; j < SUBJECTS; j++)
    {
        if(!(marks[j] >= 0 && marks[j] <= 100))
        {
            return 0;
        }
    }
    return 1;
}

int findStudent(int roll)
{
    int i;
    for(i = 0; i < count; i++)
    {
        if(list[i].roll_number == roll)
        {
            return i;
        }
    }
    return -1;
}

void loadFromFile()
{
    FILE *fp = fopen("students.txt", "r");
    char rollLine[LINE_SIZE], nameLine[LINE_SIZE], marksLine[LINE_SIZE], extra;
    student_details temp;
    int n = 0, headerOk, problems = 0, inBadStretch = 0, roll, blank;
    int first, rollRead, nameRead, marksRead;
    if(fp == NULL)
    {
        return;
    }
    first = readLine(fp, rollLine, LINE_SIZE);
    if(first == 0)
    {
        fclose(fp);
        return;
    }
    headerOk = first == 1 && strlen(rollLine) <= 9 && sscanf(rollLine, "%d %c", &n, &extra) == 1 && n >= 0 && n <= MAX;
    if(headerOk)
    {
        rollRead = readLine(fp, rollLine, LINE_SIZE);
    }
    else
    {
        rollRead = first;
        problems = 1;
        inBadStretch = 1;
    }
    nameRead = readLine(fp, nameLine, LINE_SIZE);
    marksRead = readLine(fp, marksLine, LINE_SIZE);
    while(rollRead != 0)
    {
        blank = rollRead == 1 && strspn(rollLine, " \t") == strlen(rollLine);
        if(count >= MAX && !blank)
        {
            problems++;
            break;
        }
        roll = 0;
        if(rollRead == 1 && !blank)
        {
            roll = parseRoll(rollLine);
        }
        if(roll != 0 && nameRead == 1 && marksRead == 1 && validName(nameLine) && parseMarks(marksLine, temp.marks) && findStudent(roll) == -1)
        {
            temp.roll_number = roll;
            strcpy(temp.name, nameLine);
            list[count] = temp;
            calculateResult(count);
            count++;
            inBadStretch = 0;
            rollRead = readLine(fp, rollLine, LINE_SIZE);
            nameRead = readLine(fp, nameLine, LINE_SIZE);
            marksRead = readLine(fp, marksLine, LINE_SIZE);
        }
        else
        {
            if(!blank && !inBadStretch)
            {
                problems++;
                inBadStretch = 1;
            }
            strcpy(rollLine, nameLine);
            rollRead = nameRead;
            strcpy(nameLine, marksLine);
            nameRead = marksRead;
            marksRead = readLine(fp, marksLine, LINE_SIZE);
        }
    }
    fclose(fp);
    if(headerOk && n != count && problems == 0)
    {
        problems++;
    }
    if(problems > 0)
    {
        remove("students_backup.txt");
        if(rename("students.txt", "students_backup.txt") == 0)
        {
            saveToFile();
            printf("Warning: %d problem(s) found in students.txt.\n", problems);
            printf("Valid students were loaded. Original file saved as students_backup.txt\n");
        }
        else
        {
            printf("Warning: %d problem(s) found in students.txt.\n", problems);
            printf("Valid students were loaded. Could not make a backup, so the file was left as it is.\n");
        }
    }
}

void addStudent()
{
    int i, j, roll;
    if(count >= MAX)
    {
        printf("List is full\n");
        return;
    }
    printf("Enter roll number: ");
    roll = readInt(1, 999999999);
    for(i = 0; i < count; i++)
    {
        if(list[i].roll_number == roll)
        {
            printf("Roll number already exists\n");
            return;
        }
    }
    list[count].roll_number = roll;
    printf("Enter name: ");
    readName(list[count].name, 100);
    for(j = 0; j < SUBJECTS; j++)
    {
        printf("Enter marks in %s: ", subjects[j]);
        list[count].marks[j] = readMark();
    }
    calculateResult(count);
    count++;
    saveToFile();
    printf("Student added\n");
}

void viewAll()
{
    int i;
    if(count == 0)
    {
        printf("No students yet\n");
        return;
    }
    printf("%-10s %-20s %-8s %-9s %-6s %-6s\n", "Roll", "Name", "Percent", "Backlogs", "Grade", "Status");
    for(i = 0; i < count; i++)
    {
        printf("%-10d %-20s %-8.2f %-9d %-6s %-6s\n", list[i].roll_number, list[i].name, list[i].percentage, list[i].backlogs, list[i].grade, list[i].status);
    }
}

void loadSampleData()
{
    char names[10][30] = {"Mihit Sharma", "Shlok Singh", "Ankush Das", "Koshika Bansal", "Rohit Singh", "Mohit Singh", "Somit Singh", "Purohit Singh", "Sumohit Singh", "Divyanshu Bisht"};
    int rolls[10] = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};
    float marks[10][SUBJECTS] = {
        {95, 90, 100, 100, 100},
        {78, 82, 75, 80, 85},
        {65, 70, 60, 72, 68},
        {88, 91, 84, 93, 90},
        {45, 50, 42, 55, 48},
        {35, 35, 35, 35, 32},
        {92, 95, 30, 90, 94},
        {82, 78, 80, 76, 84},
        {50, 28, 60, 30, 55},
        {96, 94, 98, 92, 95}
    };
    int i, j, added = 0;
    for(i = 0; i < 10; i++)
    {
        if(count >= MAX)
        {
            break;
        }
        if(findStudent(rolls[i]) != -1)
        {
            continue;
        }
        list[count].roll_number = rolls[i];
        strcpy(list[count].name, names[i]);
        for(j = 0; j < SUBJECTS; j++)
        {
            list[count].marks[j] = marks[i][j];
        }
        calculateResult(count);
        count++;
        added++;
    }
    saveToFile();
    printf("%d sample students added\n", added);
}

void searchStudent()
{
    int j, roll, i;
    if(count == 0)
    {
        printf("No students yet\n");
        return;
    }
    printf("Enter roll number to search: ");
    roll = readInt(1, 999999999);
    i = findStudent(roll);
    if(i == -1)
    {
        printf("Student not found\n");
        return;
    }
    printf("\nRoll number: %d\n", list[i].roll_number);
    printf("Name: %s\n", list[i].name);
    for(j = 0; j < SUBJECTS; j++)
    {
        printf("%-25s %-8.2f", subjects[j], list[i].marks[j]);
        if(list[i].marks[j] < PASS_MARK)
        {
            printf("<- BACKLOG");
        }
        printf("\n");
    }
    printf("Total: %.2f out of %d\n", list[i].total, SUBJECTS * 100);
    printf("Percentage: %.2f\n", list[i].percentage);
    printf("Backlogs: %d\n", list[i].backlogs);
    printf("Grade: %s\n", list[i].grade);
    printf("Status: %s\n\n", list[i].status);
}

void showRankList()
{
    student_details copy[MAX], temp;
    int i, j, rank = 1, firstFailed, secondFailed;
    if(count == 0)
    {
        printf("No students yet\n");
        return;
    }
    for(i = 0; i < count; i++)
    {
        copy[i] = list[i];
    }
    for(i = 0; i < count - 1; i++)
    {
        for(j = 0; j < count - 1 - i; j++)
        {
            firstFailed = copy[j].backlogs > 0;
            secondFailed = copy[j + 1].backlogs > 0;
            if(firstFailed > secondFailed || (firstFailed == secondFailed && copy[j].total < copy[j + 1].total))
            {
                temp = copy[j];
                copy[j] = copy[j + 1];
                copy[j + 1] = temp;
            }
        }
    }
    printf("%-6s %-10s %-20s %-8s %-8s %-9s %-6s\n", "Rank", "Roll", "Name", "Total", "Percent", "Backlogs", "Status");
    for(i = 0; i < count; i++)
    {
        if(i > 0 && (copy[i].total != copy[i - 1].total || (copy[i].backlogs > 0) != (copy[i - 1].backlogs > 0)))
        {
            rank = i + 1;
        }
        printf("%-6d %-10d %-20s %-8.2f %-8.2f %-9d %-6s\n", rank, copy[i].roll_number, copy[i].name, copy[i].total, copy[i].percentage, copy[i].backlogs, copy[i].status);
    }
}

void showStatistics()
{
    int i, j, passed = 0, failed = 0, top = 0, topSubject, totalBacklogs = 0, subjectFails;
    float classTotal = 0, sum;
    if(count == 0)
    {
        printf("No students yet\n");
        return;
    }
    for(i = 0; i < count; i++)
    {
        classTotal += list[i].percentage;
        totalBacklogs += list[i].backlogs;
        if(strcmp(list[i].status, "PASS") == 0)
        {
            passed++;
        }
        else
        {
            failed++;
        }
        if(list[i].total > list[top].total)
        {
            top = i;
        }
    }
    printf("\nTotal students: %d\n", count);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Total backlogs in class: %d\n", totalBacklogs);
    printf("Pass percentage: %.2f%%\n", passed * 100.0 / count);
    printf("Class average percentage: %.2f\n", classTotal / count);
    printf("Overall topper: %s (%.2f out of %d)\n\n", list[top].name, list[top].total, SUBJECTS * 100);
    printf("%-25s %-10s %-20s %-8s %-8s\n", "Subject", "Average", "Topper", "Marks", "Failed");
    for(j = 0; j < SUBJECTS; j++)
    {
        sum = 0;
        topSubject = 0;
        subjectFails = 0;
        for(i = 0; i < count; i++)
        {
            sum += list[i].marks[j];
            if(list[i].marks[j] > list[topSubject].marks[j])
            {
                topSubject = i;
            }
            if(list[i].marks[j] < PASS_MARK)
            {
                subjectFails++;
            }
        }
        printf("%-25s %-10.2f %-20s %-8.2f %-8d\n", subjects[j], sum / count, list[topSubject].name, list[topSubject].marks[j], subjectFails);
    }
    printf("\n");
}

void editStudent()
{
    int i, j, roll, choice, subject;
    if(count == 0)
    {
        printf("No students yet\n");
        return;
    }
    printf("Enter roll number to edit: ");
    roll = readInt(1, 999999999);
    i = findStudent(roll);
    if(i == -1)
    {
        printf("Student not found\n");
        return;
    }
    printf("Editing %s\n", list[i].name);
    printf("1.Change name\n2.Change marks of a subject\n0.Cancel\n");
    choice = readInt(0, 2);
    if(choice == 1)
    {
        printf("Enter new name: ");
        readName(list[i].name, 100);
    }
    else if(choice == 2)
    {
        for(j = 0; j < SUBJECTS; j++)
        {
            printf("%d.%s\n", j + 1, subjects[j]);
        }
        printf("Choose subject: ");
        subject = readInt(1, SUBJECTS);
        printf("Enter new marks in %s: ", subjects[subject - 1]);
        list[i].marks[subject - 1] = readMark();
    }
    else
    {
        return;
    }
    calculateResult(i);
    saveToFile();
    printf("Student updated\n");
}

void deleteStudent()
{
    int i, roll, confirm;
    if(count == 0)
    {
        printf("No students yet\n");
        return;
    }
    printf("Enter roll number to delete: ");
    roll = readInt(1, 999999999);
    i = findStudent(roll);
    if(i == -1)
    {
        printf("Student not found\n");
        return;
    }
    printf("Delete %s? Enter 1 to confirm, 0 to cancel: ", list[i].name);
    confirm = readInt(0, 1);
    if(confirm == 0)
    {
        printf("Cancelled\n");
        return;
    }
    for(; i < count - 1; i++)
    {
        list[i] = list[i + 1];
    }
    count--;
    saveToFile();
    printf("Student deleted\n");
}

void exportReport()
{
    FILE *fp;
    int i, j, k;
    if(count == 0)
    {
        printf("No students to export\n");
        return;
    }
    fp = fopen("report.csv", "w");
    if(fp == NULL)
    {
        printf("Could not create report.csv. If it is open in Excel, close it and try again\n");
        return;
    }
    fprintf(fp, "Roll,Name");
    for(j = 0; j < SUBJECTS; j++)
    {
        fprintf(fp, ",%s", subjects[j]);
    }
    fprintf(fp, ",Total,Percentage,Backlogs,Grade,Status\n");
    for(i = 0; i < count; i++)
    {
        fprintf(fp, "%d,\"", list[i].roll_number);
        for(k = 0; list[i].name[k] != '\0'; k++)
        {
            if(list[i].name[k] == '"')
            {
                fputc('"', fp);
            }
            fputc(list[i].name[k], fp);
        }
        fputc('"', fp);
        for(j = 0; j < SUBJECTS; j++)
        {
            fprintf(fp, ",%.2f", list[i].marks[j]);
        }
        fprintf(fp, ",%.2f,%.2f,%d,%s,%s\n", list[i].total, list[i].percentage, list[i].backlogs, list[i].grade, list[i].status);
    }
    fclose(fp);
    printf("Report saved as report.csv (%d students)\n", count);
}

int main()
{
    int choice;
    loadFromFile();
    do{
        printf("MENU\n");
        printf("1.Add\n2.View All\n3.Search\n4.Rank List\n5.Statistics\n6.Edit\n7.Delete\n8.Load Sample Data\n9.Export Report\n0.Exit\n");
        choice = readInt(0, 9);
        switch(choice)
        {
            case 0:
            printf("Program Exited\n");
            break;
            case 1:
            addStudent();
            break;
            case 2:
            viewAll();
            break;
            case 3:
            searchStudent();
            break;
            case 4:
            showRankList();
            break;
            case 5:
            showStatistics();
            break;
            case 6:
            editStudent();
            break;
            case 7:
            deleteStudent();
            break;
            case 8:
            loadSampleData();
            break;
            case 9:
            exportReport();
            break;
            default:
            printf("Invalid choice\n");
            break;
        }
    }while(choice!=0);
    return 0;
}