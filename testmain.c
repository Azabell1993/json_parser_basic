#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "user.h"

int main() {
    printf("Press Enter to continue...\n");
    while (getchar() != '\n');  // Enter 입력을 대기

    // UserDB 초기화 및 JSON 파일 읽기
    UserDB user_db;
    init_user_db(&user_db);

    // JSON 파일 읽기
    char *json_data = read_json_file(JSON_FILE);
    if (json_data != NULL) {
        printf("JSON 파일 읽기 성공\n");
    } else {
        printf("JSON 파일 읽기 실패\n");
        return 1;
    }

    // JSON 데이터에서 사용자 정보 파싱
    if (parse_user_info(&user_db, json_data)) {
        printf("Users parsed successfully.\n");
    } else {
        printf("Error parsing user information.\n");
        free(json_data);
        return 1;
    }

    char choice_str[MAX_STRING_SIZE];
    int choice;
    char host[MAX_STRING_SIZE], user[MAX_STRING_SIZE], pass[MAX_STRING_SIZE], name[MAX_STRING_SIZE];

    // 사용자 메뉴 처리
    while (1) {
        printf("\n1. Register User\n2. Query User\n3. Delete User\n4. Display All Users\n5. Exit\n");
        printf("Enter your choice or END : [ exit | ... ] ");
        scanf("%s", choice_str);  // 문자열로 입력을 받음

        // "exit" 또는 "..."가 입력되면 서버 종료
        if (strcmp(choice_str, "exit") == 0) {
            printf("서버 종료\n");
            break;
        } else if (strcmp(choice_str, "...") == 0) {
            printf("클라이언트 종료\n");
            break;
        }

        // 문자열을 정수로 변환하여 choice 처리
        choice = atoi(choice_str);  // 문자열을 정수로 변환

        switch (choice) {
            case 1:  // 사용자 등록
                printf("Enter host: ");
                scanf("%s", host);
                printf("Enter user: ");
                scanf("%s", user);
                printf("Enter password: ");
                scanf("%s", pass);
                printf("Enter name: ");
                scanf("%s", name);
                register_user(&user_db, host, user, pass, name);
                break;

            case 2:  // 사용자 조회
                printf("Enter username to query: ");
                scanf("%s", user);

                // 로그인할 때도 활용이 가능하다.
                query_user(&user_db, user, pass);
                break;

            case 3:  // 사용자 삭제
                printf("Enter username to delete: ");
                scanf("%s", user);
                delete_user(&user_db, user); 
                break;

            case 4:  // 모든 사용자 출력
                display_all_users(&user_db);
                break;

            case 5:  // 종료
                free(json_data);
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

    free(json_data);  // JSON 데이터 메모리 해제
    return 0;
}
