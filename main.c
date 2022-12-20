#include <stdio.h>
#include <string.h>
#include <time.h>

int main()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf("%d\n", tm.tm_year + 1900);
    char year[5];
    sprintf(year, "%d", tm.tm_year + 1900);
    printf("%s\n", year);
    char month[3];
    sprintf(month, "%d", tm.tm_mon + 1);
    printf("%s\n", month);
    char day[3];
    sprintf(day, "%d", tm.tm_mday);
    printf("%s\n", day);
    char hour[3];
    sprintf(hour, "%d", tm.tm_hour);
    printf("%s\n", hour);
    char min[3];
    sprintf(min, "%d", tm.tm_min);
    printf("%s\n", min);
    char sec[3];
    sprintf(sec, "%d", tm.tm_sec);
    printf("%s\n", sec);
    char date[20];
    strcpy(date, year);
    strcat(date, "-");
    strcat(date, month);
    strcat(date, "-");
    strcat(date, day);
    strcat(date, " ");
    strcat(date, hour);
    strcat(date, ":");
    strcat(date, min);
    strcat(date, ":");
    strcat(date, sec);
    printf("%s\n", date);

    return 0;
}