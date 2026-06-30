#ifndef H_INC_WLAN__P9ERROR_H
#define H_INC_WLAN__P9ERROR_H

#include <types.h>

typedef uint64_t    jmp_buf[14];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val) NORETURN;


struct error_stack_t {
#define ERROR_STACK_SIZE    30
    jmp_buf stack[ERROR_STACK_SIZE];
    volatile unsigned stackptr;
};

struct error_stack_t *get_error_stack (void);

void p9error(const char *str);
#define Eio         "I/O error"
#define Enomem      "Not enough memory"
#define Enonexist   "File does not exist"

jmp_buf *pusherror(void);
#define waserror()    setjmp(*pusherror())

void nexterror(void);
void poperror(void);

void okay(int status);

#endif
