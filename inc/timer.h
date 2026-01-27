#ifndef INC_TIMER_H
#define INC_TIMER_H

#include <types.h>
#include <linux/time.h>

void timer_init();
void timer_intr(int user_mode);

struct timer_list *alloc_timer(void);
void free_timer(struct timer_list *timer);

void add_timer(struct timer_list *timer);
int  del_timer(struct timer_list *timer, boolean del);
void run_timer_list(void);

#endif
