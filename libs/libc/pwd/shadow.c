#include <assert.h>
#include <shadow.h>
#include <stdio.h>
#include <string.h>

static FILE* spwdfile = NULL;
static char tmp_buf[512];

// spwd struct contains pointers to data which is
// stored in spwd_buf
static char spwd_buf[512];
static spwd_t spwd_entry;

static inline int spwd_part_to_int(char* part)
{
    int res = 0;
    sscanf(part, "%d", &res);
    return res;
}

static inline uint32_t spwd_part_to_uint(char* part)
{
    uint32_t res = 0;
    sscanf(part, "%u", &res);
    return res;
}

static spwd_t* parse_spwd_entry(char* res)
{
    int len = strlen(res);
    int nxt_part = 0;
    char* parts[10];

    memcpy(spwd_buf, res, len);

    parts[nxt_part++] = &spwd_buf[0];
    for (int i = 0; i < len; i++) {
        if (spwd_buf[i] == ':') {
            parts[nxt_part++] = &spwd_buf[i + 1];
            spwd_buf[i] = '\0';
            if (nxt_part > 9) {
                // More than 9 block.
                return NULL;
            }
        }
    }
    spwd_buf[len] = '\0';

    if (nxt_part != 9) {
        return NULL;
    }

    spwd_entry.sp_namp = parts[0];
    spwd_entry.sp_pwdp = parts[1];
    spwd_entry.sp_lstchg = spwd_part_to_int(parts[2]);
    spwd_entry.sp_min = spwd_part_to_int(parts[3]);
    spwd_entry.sp_max = spwd_part_to_int(parts[4]);
    spwd_entry.sp_warn = spwd_part_to_int(parts[5]);
    spwd_entry.sp_inact = spwd_part_to_int(parts[6]);
    spwd_entry.sp_expire = spwd_part_to_int(parts[7]);
    spwd_entry.sp_flag = spwd_part_to_uint(parts[8]);
    return &spwd_entry;
}

void setspent()
{
    spwdfile = fopen("/etc/shadow", "r");
    if (spwdfile == NULL) {
        // Raise an error
    }
}

void endspent()
{
    if (spwdfile) {
        fclose(spwdfile);
        spwdfile = NULL;
    }
    memset(&spwd_entry, 0, sizeof(spwd_entry));
}

spwd_t* getspent()
{
    if (!spwdfile) {
        setspent();
    }

    while (1) {
        if (feof(spwdfile)) {
            return NULL;
        }

        char* res = fgets(spwd_buf, 512, spwdfile);
        if (!res || !res[0]) {
            return NULL;
        }

        if (parse_spwd_entry(res)) {
            return &spwd_entry;
        }
    }

    return NULL;
}

spwd_t* getspnam(const char* name)
{
    spwd_t* spwd;
    setspent();
    while ((spwd = getspent())) {
        if (!strcmp(spwd->sp_namp, name)) {
            return spwd;
        }
    }
    return NULL;
}