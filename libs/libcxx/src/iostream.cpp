#include <__config>
#include <__std_streambuffer>
#include <cstdio>
#include <new>
#include <ostream>

_LIBCXX_BEGIN_NAMESPACE_STD

_ALIGNAS_TYPE(ostream)
char cout[sizeof(ostream)];
_ALIGNAS_TYPE(__stdoutbuf<char>)
static char cout_buf[sizeof(__stdoutbuf<char>)];
static ostream* cout_ptr;

_ALIGNAS_TYPE(ostream)
char cerr[sizeof(ostream)];
_ALIGNAS_TYPE(__stdoutbuf<char>)
static char cerr_buf[sizeof(__stdoutbuf<char>)];
static ostream* cerr_ptr;

int _ios_init()
{
    cout_ptr = new (cout) ostream(new (cout_buf) __stdoutbuf<char>(stdout));
    cerr_ptr = new (cerr) ostream(new (cerr_buf) __stdoutbuf<char>(stderr));
    return 0;
}

int _ios_deinit()
{
    cout_ptr->flush();
    cerr_ptr->flush();
    return 0;
}

_LIBCXX_END_NAMESPACE_STD