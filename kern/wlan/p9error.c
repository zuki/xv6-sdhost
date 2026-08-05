#include <types.h>
#include <wlan/p9error.h>
#include <wlan/p9proc.h>
#include <wlan/p9util.h>
#include <proc.h>

// エラーをupにセットしてエラースタックの一番上にあるjmp_bufにlong_jmpする
void p9error(const char *str)
{
    print("p0err: %s\n", str);
    up->errstr = str;

    struct error_stack_t *s = get_error_stack();
    assert(s != 0);

    assert(s->stackptr < ERROR_STACK_SIZE);
    s->stackptr++;
    //print("[%d] err: %d\n", thisproc()->pid, s->stackptr);
    longjmp(s->stack[s->stackptr-1], 1);
}

// エラースタックの一番上にあるjmp_bufを返す
jmp_buf *pusherror(void)
{
    struct error_stack_t *s = get_error_stack ();
    assert (s != 0);
    //print("[%d] push: %d\n", thisproc()->pid, s->stackptr - 1);
    return &s->stack[--s->stackptr];
}

// エラースタックからエラーを取り出して返す
void nexterror(void)
{
    struct error_stack_t *s = get_error_stack ();
    assert (s != 0);

    assert (s->stackptr < ERROR_STACK_SIZE);
    s->stackptr++;
    //print("[%d] next: %d\n", thisproc()->pid, s->stackptr);
    longjmp (s->stack[s->stackptr-1], 1);
}

// エラースタックからエラーを1つ取り出して捨てる
void poperror(void)
{
    struct error_stack_t *s = get_error_stack ();
    assert (s != 0);

    assert (s->stackptr < ERROR_STACK_SIZE);
    s->stackptr++;
    //print("[%d] pop: %d\n", thisproc()->pid, s->stackptr);
}

// 何もしない
void okay (int status)
{
}
