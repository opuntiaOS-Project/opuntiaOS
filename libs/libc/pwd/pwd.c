#include <assert.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>

static FILE* pwdfile = NULL;
static char tmp_buf[512];

// Passwd struct contains pointers to data which is
// stored in passwd_buf
static char passwd_buf[512];
static passwd_t passwd_entry;

static passwd_t* parse_passwd_entry(char* res)
{
    int len = strlen(res);
    int nxt_part = 0;
    char* parts[8];

    memcpy(passwd_buf, res, len);

    parts[nxt_part++] = &passwd_buf[0];
    for (int i = 0; i < len; i++) {
        if (passwd_buf[i] == ':') {
            parts[nxt_part++] = &passwd_buf[i + 1];
            passwd_buf[i] = '\0';
            if (nxt_part > 7) {
                // More than 7 block.
                return NULL;
            }
        }
    }
    passwd_buf[len] = '\0';

    if (nxt_part != 7) {
        return NULL;
    }

    uid_t uid;
    gid_t gid;
    sscanf(parts[2], "%u", &uid);
    sscanf(parts[3], "%u", &gid);

    passwd_entry.pw_name = parts[0];
    passwd_entry.pw_passwd = parts[1];
    passwd_entry.pw_uid = uid;
    passwd_entry.pw_gid = gid;
    passwd_entry.pw_gecos = parts[4];
    passwd_entry.pw_dir = parts[5];
    passwd_entry.pw_shell = parts[6];

    return &passwd_entry;
}

void setpwent()
{
    pwdfile = fopen("/etc/passwd", "r");
    if (pwdfile == NULL) {
        // Raise an error
    }
}

void endpwent()
{
    if (pwdfile) {
        fclose(pwdfile);
        pwdfile = NULL;
    }
    memset(&passwd_entry, 0, sizeof(passwd_entry));
}

passwd_t* getpwent()
{
    if (!pwdfile) {
        setpwent();
    }

    while (1) {
        if (feof(pwdfile)) {
            return NULL;
        }

        char* res = fgets(passwd_buf, 512, pwdfile);
        if (!res || !res[0]) {
            return NULL;
        }

        if (parse_passwd_entry(res)) {
            return &passwd_entry;
        }
    }

    return NULL;
}

passwd_t* getpwuid(uid_t uid)
{
    passwd_t* passwd;
    setpwent();
    while ((passwd = getpwent())) {
        if (passwd->pw_uid == uid) {
            return passwd;
        }
    }
    return NULL;
}

passwd_t* getpwnam(const char* name)
{
    passwd_t* passwd;
    setpwent();
    while ((passwd = getpwent())) {
        if (!strcmp(passwd->pw_name, name)) {
            return passwd;
        }
    }
    return NULL;
}