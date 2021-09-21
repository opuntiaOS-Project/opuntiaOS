#include <fcntl.h>
#include <pwd.h>
#include <shadow.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char pswd[64];

int auth(char* uname)
{
    spwd_t* spwd;
    spwd = getspnam(uname);
    if (spwd == NULL) {
        return -1;
    }

    printf("Password: ");
    fflush(stdout);
    scanf("%s", pswd);
    int res = strcmp(pswd, spwd->sp_pwdp);
    endspent();
    return res;
}

int main(int argc, char** argv)
{
    if (auth("root") != 0) {
        printf("Incorrect password\n");
        return -1;
    }

    // Sign as root
    passwd_t* pwd = getpwnam("root");
    if (pwd == NULL) {
        printf("unable to retrieve user info");
        return -1;
    }

    if (setuid(pwd->pw_uid) != 0) {
        printf("setuid failure");
        return -1;
    }

    execve(argv[1], &argv[2], NULL);
    return 0;
}