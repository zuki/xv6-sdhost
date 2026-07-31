#include <types.h>
#include <wlan/p9error.h>
#include <wlan/p9proc.h>
#include <wlan/p9util.h>

// エラーをupにセットしてエラースタックの一番上にあるjmp_bufにlong_jmpする
void p9error(const char *str)
{
    print("%s\n", str);
    up->errstr = str;

    struct error_stack_t *s = get_error_stack();
    assert(s != 0);

    assert(s->stackptr < ERROR_STACK_SIZE);
    s->stackptr++;
    longjmp(s->stack[s->stackptr-1], 1);
}

// エラースタックの一番上にあるjmp_bufを返す
jmp_buf *pusherror(void)
{
    struct error_stack_t *s = get_error_stack ();
    assert (s != 0);

    return &s->stack[--s->stackptr];
}

// エラースタックからエラーを取り出して返す
void nexterror(void)
{
    struct error_stack_t *s = get_error_stack ();
    assert (s != 0);

    assert (s->stackptr < ERROR_STACK_SIZE);
    s->stackptr++;
    longjmp (s->stack[s->stackptr-1], 1);
}

// エラースタックからエラーを1つ取り出して捨てる
void poperror(void)
{
    struct error_stack_t *s = get_error_stack ();
    assert (s != 0);
    if (s->stackptr >= ERROR_STACK_SIZE) {
        debug("s->stackptr: %d", s->stackptr);
    }
    assert (s->stackptr < ERROR_STACK_SIZE);
    s->stackptr++;
}

// 何もしない
void okay (int status)
{
}
