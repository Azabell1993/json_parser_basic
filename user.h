#pragma ONCE
#ifndef USER_REGISTRATION_H
#define USER_REGISTRATION_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include "smartptr.h"

#define MAX_STRING_SIZE 100
#define MAX_USERS 10
#define JSON_FILE "userinfo.json"

typedef struct {
    SmartPtr host;
    SmartPtr user;
    SmartPtr pass;
    SmartPtr name;
} UserInfo;

typedef struct {
    UserInfo users[MAX_USERS];
    size_t user_count;
    pthread_mutex_t db_mutex;
} UserDB;

void init_user_db(UserDB *db);
void write_json_file(UserDB *db);
void display_all_users(UserDB *db);
int query_user(UserDB *db, const char *username, const char *password);
int query_userId(UserDB *db, const char *username);
bool register_user(UserDB *db, const char *host, const char *user, const char *pass, const char *name);
bool delete_user(UserDB *db, const char *username);
bool parse_user_info(UserDB *db, const char *json_data);
char *read_json_file(const char *filename);


void init_user_db(UserDB *db) {
    db->user_count = 0;
    pthread_mutex_init(&db->db_mutex, NULL);
}

void write_json_file(UserDB *db) {
    FILE *file = fopen(JSON_FILE, "w");
    if (!file) {
        printf("Unable to open the file: %s\n", JSON_FILE);
        return;
    }

    fprintf(file, "[\n");
    for (size_t i = 0; i < db->user_count; ++i) {
        fprintf(file, "  {\n");
        fprintf(file, "    \"host\": \"%s\",\n", (char *)db->users[i].host.ptr);
        fprintf(file, "    \"user\": \"%s\",\n", (char *)db->users[i].user.ptr);
        fprintf(file, "    \"pass\": \"%s\",\n", (char *)db->users[i].pass.ptr);
        fprintf(file, "    \"name\": \"%s\"\n", (char *)db->users[i].name.ptr);
        if (i == db->user_count - 1) {
            fprintf(file, "  }\n");
        } else {
            fprintf(file, "  },\n");
        }
    }
    fprintf(file, "]\n");

    fclose(file);
    printf("User data saved to %s\n", JSON_FILE);
}

int query_userId(UserDB *db, const char *username) {
    pthread_mutex_lock(&db->db_mutex);

    for (size_t i = 0; i < db->user_count; ++i) {
        if (strcmp((char *)db->users[i].user.ptr, username) == 0) {
            pthread_mutex_unlock(&db->db_mutex);
            return i;
        } else {
            pthread_mutex_unlock(&db->db_mutex);
            return -1;
        }
    }

    pthread_mutex_unlock(&db->db_mutex);
    return -1;
}

int query_user(UserDB *db, const char *username, const char *password) {
    pthread_mutex_lock(&db->db_mutex);

    for (size_t i = 0; i < db->user_count; ++i) {
        if (strcmp((char *)db->users[i].user.ptr, username) == 0) {
            if (strcmp((char *)db->users[i].pass.ptr, password) == 0) {
                pthread_mutex_unlock(&db->db_mutex);
                return i;
            } else {
                pthread_mutex_unlock(&db->db_mutex);
                return -1;
            }
        }
    }

    pthread_mutex_unlock(&db->db_mutex);
    return -1;
}

bool register_user(UserDB *db, const char *host, const char *user, const char *pass, const char *name) {
    pthread_mutex_lock(&db->db_mutex);

    if (db->user_count >= MAX_USERS) {
        printf("User database is full.\n");
        pthread_mutex_unlock(&db->db_mutex);
        return false;
    }

    UserInfo new_user;
    new_user.host = CREATE_SMART_PTR(char[MAX_STRING_SIZE], host);
    new_user.user = CREATE_SMART_PTR(char[MAX_STRING_SIZE], user);
    new_user.pass = CREATE_SMART_PTR(char[MAX_STRING_SIZE], pass);
    new_user.name = CREATE_SMART_PTR(char[MAX_STRING_SIZE], name);

    retain(&new_user.host);
    retain(&new_user.user);
    retain(&new_user.pass);
    retain(&new_user.name);

    db->users[db->user_count++] = new_user;

    pthread_mutex_unlock(&db->db_mutex);
    printf("User registered successfully.\n");

    // Save the updated database to the JSON file
    write_json_file(db);
    return true;
}

bool delete_user(UserDB *db, const char *username) {
    pthread_mutex_lock(&db->db_mutex);

    for (size_t i = 0; i < db->user_count; ++i) {
        if (strcmp((char *)db->users[i].user.ptr, username) == 0) {
            printf("Deleting user: %s\n", username);

            // Free smart pointer memory
            release(&db->users[i].host);
            release(&db->users[i].user);
            release(&db->users[i].pass);
            release(&db->users[i].name);

            // Shift users down
            for (size_t j = i; j < db->user_count - 1; ++j) {
                db->users[j] = db->users[j + 1];
            }

            db->user_count--;
            pthread_mutex_unlock(&db->db_mutex);
            printf("User deleted successfully.\n");

            // Save the updated database to the JSON file
            write_json_file(db);
            return true;
        }
    }

    pthread_mutex_unlock(&db->db_mutex);
    printf("User not found.\n");
    return false;
}

char *read_json_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Unable to open the file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char *json_data = (char *)malloc(filesize + 1);
    fread(json_data, 1, filesize, file);
    json_data[filesize] = '\0';

    fclose(file);
    return json_data;
}

bool parse_user_info(UserDB *db, const char *json_data) {
    char buffer[MAX_STRING_SIZE];
    const char *current = json_data;

    while (*current) {
        if (*current == '{') {
            UserInfo new_user;
            memset(&new_user, 0, sizeof(UserInfo));

            while (*current && *current != '}') {
                current++;
                if (strncmp(current, "\"host\":", 7) == 0) {
                    current += 7;
                    sscanf(current, "\"%[^\"]\"", buffer);
                    new_user.host = CREATE_SMART_PTR(char[MAX_STRING_SIZE], buffer);
                    retain(&new_user.host);
                } else if (strncmp(current, "\"user\":", 7) == 0) {
                    current += 7;
                    sscanf(current, "\"%[^\"]\"", buffer);
                    new_user.user = CREATE_SMART_PTR(char[MAX_STRING_SIZE], buffer);
                    retain(&new_user.user);
                } else if (strncmp(current, "\"pass\":", 7) == 0) {
                    current += 7;
                    sscanf(current, "\"%[^\"]\"", buffer);
                    new_user.pass = CREATE_SMART_PTR(char[MAX_STRING_SIZE], buffer);
                    retain(&new_user.pass);
                } else if (strncmp(current, "\"name\":", 7) == 0) {
                    current += 7;
                    sscanf(current, "\"%[^\"]\"", buffer);
                    new_user.name = CREATE_SMART_PTR(char[MAX_STRING_SIZE], buffer);
                    retain(&new_user.name);
                }
            }

            if (db->user_count < MAX_USERS) {
                db->users[db->user_count++] = new_user;
            } else {
                printf("User database is full.\n");
                return false;
            }
        }
        current++;
    }
    return true;
}

void display_all_users(UserDB *db) {
    pthread_mutex_lock(&db->db_mutex);

    printf("Displaying all users:\n");
    for (size_t i = 0; i < db->user_count; ++i) {
        printf("Host: %s, User: %s, Pass: %s, Name: %s\n",
            (char *)db->users[i].host.ptr,
            (char *)db->users[i].user.ptr,
            (char *)db->users[i].pass.ptr,
            (char *)db->users[i].name.ptr);
    }

    pthread_mutex_unlock(&db->db_mutex);
}

#endif // USER_REGISTRATION_H
