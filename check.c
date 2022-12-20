// connect to mysql
#include <stdlib.h>
#include <string.h>
#include "checkTime.h"
#include "linkedList.h"
#include "fetch_data.h"

node head = NULL;

void menu()
{
    printf("Thao tac voi database:\n");
    printf("1. Xem danh sach cac tables\n");
    printf("2. Xem danh sach nguoi dung trong database\n");
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
    char *passwordDB = "26072001";
    char *database = "test";
    conn = mysql_init(NULL);
    char username[100];
    char password[100];
    int check = 1;
    int isLogin = 0;
    int choice, idLogin;
    if (!mysql_real_connect(conn, server, user, passwordDB, database, 0, NULL, 0))
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
            strcpy(query, "select * from user");
            // strcat(query, table);
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
                head = addTail(head, atoi(row[0]), row[1], row[2], atoi(row[3]), row[4], row[5]);
            }
            printList(head);
            break;
        }
        case 3:
        {
            if (isLogin == 1)
            {
                printf("Ban dang dang nhap\n");
                printf("Vui long dang xuat de dang nhap lai\n");
            }
            else
            {
                int valCheck = 3;
                int count = 1;
                while (count == 1)
                {
                    printf("Nhap username: ");
                    scanf("%s", username);
                    char query[100];
                    strcpy(query, "select id,password,status from user where username = '");
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
                                    isLogin = 1;
                                    idLogin = atoi(row[0]);
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
            }
            break;
        }
        case 4:
        {
            if (isLogin == 1)
            {
                printf("Ban dang dang nhap\n");
                printf("Vui long dang xuat de dang ki?\n");
            }
            else
            {
                char usernameNew[100];

                printf("Nhap username: ");
                scanf("%s", usernameNew);
                char query[100];
                strcpy(query, "select id from user where username = '");
                strcat(query, usernameNew);
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
                    char lastLogin[100];
                    char name[100];
                    printf("Nhap password: ");
                    scanf("%s", password);
                    // printf("Nhap lastLogin: ");
                    // scanf("%s", lastLogin);
                    printf("Nhap ho ten day du: ");
                    scanf("%s", name);
                    char query[100];
                    strcpy(query, "insert into user(userName, password, status, name) values('");
                    strcat(query, usernameNew);
                    strcat(query, "', '");
                    strcat(query, password);
                    strcat(query, "', ");
                    strcat(query, "2, '");
                    // strcat(query, lastLogin);
                    // strcat(query, "', '");
                    strcat(query, name);
                    strcat(query, "')");
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
            }
            break;
        }
        case 5:
        {
            if (isLogin == 0)
            {
                printf("Ban chua dang nhap!!!\n");
            }
            else
            {
                printf("Ban co muon dang xuat khoi trai dat ? \n");
                printf("1. Co \t 2. Khong \n");
                int checkDx;
                scanf("%d", &checkDx);
                if (checkDx == 1)
                {
                    checkTime();
                    char sidLogin[100];
                    sprintf(sidLogin, "%d", idLogin);

                    char query[100];
                    strcpy(query, "update user set lastLogin = '");
                    strcat(query, date);
                    strcat(query, "' where id = ");
                    strcat(query, sidLogin);
                    printf("query: %s \n", query);
                    mysql_free_result(res);
                    if (mysql_query(conn, query))
                    {
                        fprintf(stderr, "%s \n", mysql_error(conn));
                        exit(1);
                    }
                    res = mysql_use_result(conn);
                    isLogin = 0;
                    printf("Dang xuat thanh cong \n");
                }
            }
            break;
        }
        default:
        {
            check = 0;
            break;
        }
        }
    }

    // free(query);
    // free(date);
    // free(username);
    // free(password);
    free(head);
    mysql_free_result(res);
    mysql_close(conn);
    return 0;
}
