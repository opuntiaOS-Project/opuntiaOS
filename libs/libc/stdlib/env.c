#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void free_var(const char* string)
{
    // TODO: Implement me!
}

char* getenv(const char* name)
{
    size_t len = strlen(name);
    for (int i = 0; environ[i]; i++) {
        const char* envv = environ[i];
        char* eqpos = strchr(envv, '=');
        if (!eqpos) {
            continue;
        }
        size_t curlen = (size_t)(eqpos - envv);
        if (curlen == len && strncmp(name, envv, len)) {
            return (char*)&envv[len + 1];
        }
    }

    return NULL;
}

int setenv(const char* name, const char* value, int overwrite)
{
    if (!overwrite && getenv(name)) {
        return 0;
    }

    int name_len = strlen(name);
    int value_len = strlen(value);
    int len = name_len + 1 + value_len;
    char* string = (char*)malloc(len + 1);
    strncpy(string, name, name_len);
    string[name_len] = '=';
    strncpy(&string[name_len + 1], value, value_len);
    string[len] = '\0';
    return putenv(string);
}

int putenv(char* string)
{
    char* pos = strchr(string, '=');
    if (!pos) {
        return unsetenv(string);
    }

    size_t len = (size_t)(pos - string);
    int env_i = 0;
    for (; environ[env_i]; env_i++) {
        const char* envv = environ[env_i];
        char* eqpos = strchr(envv, '=');
        if (!eqpos) {
            continue;
        }
        size_t curlen = (size_t)(eqpos - envv);
        if (len != curlen) {
            continue;
        }

        if (strncmp(string, envv, len) == 0) {
            free_var(envv);
            environ[env_i] = string;
            return 0;
        }
    }

    char** new_environ = (char**)malloc(sizeof(char*) * (env_i + 2));
    for (int i = 0; environ[i]; i++) {
        new_environ[i] = environ[i];
    }
    new_environ[env_i] = string;
    new_environ[env_i] = NULL;

    extern int __environ_malloced;
    if (__environ_malloced) {
        free(environ);
    }
    environ = new_environ;
    __environ_malloced = 1;
    return 0;
}

int unsetenv(const char* name)
{
    size_t len = strlen(name);
    int env_i = 0;
    int remove_pos = 0;
    const char* remove_env = NULL;
    for (; environ[env_i]; env_i++) {
        const char* envv = environ[env_i];
        char* eqpos = strchr(envv, '=');
        if (!eqpos) {
            continue;
        }
        size_t curlen = (size_t)(eqpos - envv);
        if (len != curlen) {
            continue;
        }

        if (strncmp(name, envv, len) == 0) {
            remove_pos = env_i;
            remove_env = envv;
        }
    }

    size_t move_len = ((env_i - 1) - remove_pos) * sizeof(char*);
    memmove(&environ[remove_pos], &environ[remove_pos + 1], move_len);
    free_var(remove_env);
    return 0;
}
