// connect to mysql
#include <stdlib.h>
#include <string.h>
#include "fetch_data.h"

void menu()
{
    printf("Thao tac voi database:\n");
    printf("1. Xem danh sach cac tables\n");
    printf("2. Xem cac table trong database\n");
    printf("3. Dang nhap\n");
    printf("4. Dang ki\n");
    printf("5. Dang xuat\n");
    printf("6. Thoat\n");
}

int main(int argc, char *argv[])
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *server = "localhost";
    char *user = "root";
    char *password = "26072001";
    char *database = "test";
    conn = mysql_init(NULL);
    int check = 1;
    int choice;
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        fprintf(stderr, "%s \n", mysql_error(conn));
        exit(1);
    }

    while (check)
    {
        menu();
        printf("Nhap lua chon: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
        {
            mysql_free_result(res);
            if (mysql_query(conn, "show tables"))
            {
                fprintf(stderr, "%s \n", mysql_error(conn));
                exit(1);
            }
            res = mysql_use_result(conn);
            printf("MySQL Tables in mysql tables: \n");
            while ((row = mysql_fetch_row(res)) != NULL)
                printf("%s \n", row[0]);
            break;
        }
        case 2:
        {
            char query[100];
            char table[100];
            printf("Nhap ten table: ");
            scanf("%s", table);
            strcpy(query, "select * from ");
            strcat(query, table);
            mysql_free_result(res);
            if (mysql_query(conn, query))
            {
                fprintf(stderr, "%s \n", mysql_error(conn));
                exit(1);
            }
            res = mysql_use_result(conn);
            printf("MySQL Tables in mysql database: \n");
            while ((row = mysql_fetch_row(res)) != NULL)
            {
                printf("STT: %s \n", row[0]);
                printf("Ten nguoi dung: %s \n", row[1]);
                printf("Mat khau: %s \n", row[2]);
                printf("Trang thai: %s \n", row[3]);
            }

            break;
        }
        case 3:
        {
            char username[100];
            char password[100];
            int valCheck = 3;
            int count = 1;
            while (count == 1)
            {
                printf("Nhap username: ");
                scanf("%s", username);
                char query[100];
                strcpy(query, "select id,pass,status from user where username = '");
                strcat(query, username);
                strcat(query, "'");
                mysql_free_result(res);
                if (mysql_query(conn, query))
                {
                    fprintf(stderr, "%s \n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                if ((row = mysql_fetch_row(res)) != NULL)
                {
                    // printf("huhuhuhuu %s\n", row[2]);
                    if (strcmp(row[2], "0") == 0)
                    {
                        printf("Tai khoan da bi khoa\n");
                        break;
                    }
                    else
                    {
                        while (valCheck > 0)
                        {
                            printf("Nhap password: ");
                            scanf("%s", password);
                            printf("row[0]: %s \n", row[0]);
                            printf("row[1]: %s \n", row[1]);

                            if (strcmp(password, row[1]) == 0)
                            {
                                printf("Dang nhap thanh cong \n");
                                valCheck = -1;
                                count = 0;
                                // update value of status
                                strcpy(query, "update user set status = 1 where id = ");
                                strcat(query, row[0]);
                                printf("query: %s \n", query);
                                mysql_free_result(res);
                                if (mysql_query(conn, query))
                                {
                                    fprintf(stderr, "%s \n", mysql_error(conn));
                                    exit(1);
                                }
                                res = mysql_use_result(conn);
                            }
                            else
                            {
                                printf("Sai mat khau\n");
                                valCheck = --valCheck;
                            }
                            printf("chekkkkk: %d\n", valCheck);
                        }
                        if (valCheck == 0)
                        {
                            printf("Tai khoan cua ban da bi khoa\n");
                            count = 0;
                            strcpy(query, "update user set status = 0 where id = ");
                            strcat(query, row[0]);
                            printf("query: %s \n", query);
                            mysql_free_result(res);
                            if (mysql_query(conn, query))
                            {
                                fprintf(stderr, "%s \n", mysql_error(conn));
                                exit(1);
                            }
                            res = mysql_use_result(conn);
                        }
                    }
                }
                else
                {
                    printf("Ten dang nhap khong ton tai!!!\n");
                    printf("Yeu cau nhap lai\n");
                }
            }
            break;
        }
        case 4:
        {
            char username[100];

            printf("Nhap username: ");
            scanf("%s", username);
            char query[100];
            strcpy(query, "select id from user where username = '");
            strcat(query, username);
            strcat(query, "'");
            mysql_free_result(res);
            if (mysql_query(conn, query))
            {
                fprintf(stderr, "%s \n", mysql_error(conn));
                exit(1);
            }
            res = mysql_use_result(conn);
            if ((row = mysql_fetch_row(res)) != NULL)
            {
                printf("Ten dang nhap da ton tai!!!\n");
            }
            else
            {
                char password[100];
                char status[100];
                printf("Nhap password: ");
                scanf("%s", password);
                // printf("Nhap trang thai: ");
                // scanf("%s", status);
                char query[100];
                strcpy(query, "insert into user(userName, pass, status) values('");
                strcat(query, username);
                strcat(query, "', '");
                strcat(query, password);
                strcat(query, "', ");
                // strcat(query, status);
                strcat(query, "2)");

                printf("query: %s \n", query);
                mysql_free_result(res);
                if (mysql_query(conn, query))
                {
                    fprintf(stderr, "%s \n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("Dang ki thanh cong \n");
            }
            break;
        }
        case 5:
        {
            printf("Dang xuat thanh cong \n");
            break;
        }
        default:
        {
            check = 0;
            break;
        }
        }
    }
    mysql_free_result(res);
    mysql_close(conn);
    return 0;
}
