#ifndef INC_TIMER_H
#define INC_TIMER_H

#include <linux/time.h>

void timer_init();
void timer_intr();

struct timer_list *alloc_timer(void);
void free_timer(struct timer_list *timer);

void add_timer(struct timer_list *timer);
int  del_timer(struct timer_list *timer);
void run_timer_list(void);

#endif
