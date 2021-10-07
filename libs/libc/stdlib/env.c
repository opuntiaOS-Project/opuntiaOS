#include <stdlib.h>
#include <string.h>

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
