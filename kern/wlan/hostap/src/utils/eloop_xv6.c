/*
 * イベントループ - 空のテンプレート（基本的な構造のみ、OS固有の操作は含まない）
 * Copyright (c) 2002-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <utils/includes.h>

#include <utils/common.h>
#include <utils/eloop.h>
#include <utils/wpa_debug.h>

extern int l2_packet_auth_active (void);

// eloopソケットイベント構造体
struct eloop_sock {
    int sock;
    void *eloop_data;
    void *user_data;
    eloop_sock_handler handler;
};

// eloopタイムアウトイベント構造体
struct eloop_timeout {
    struct os_reltime time;
    void *eloop_data;
    void *user_data;
    eloop_timeout_handler handler;
    struct eloop_timeout *next;
};

// eloopデータ構造体
struct eloop_data {
    int max_sock, reader_count;
    struct eloop_sock *readers;

    struct eloop_timeout *timeout;

    int terminate;
    int reader_table_changed;
};

static struct eloop_data eloop;


int eloop_init(void)
{
    memset(&eloop, 0, sizeof(eloop));
    return 0;
}


int eloop_register_read_sock(int sock, eloop_sock_handler handler,
                 void *eloop_data, void *user_data)
{
    struct eloop_sock *tmp;

    tmp = (struct eloop_sock *)
        kmrealloc(eloop.readers,
            (eloop.reader_count + 1) * sizeof(struct eloop_sock), sizeof(*eloop.readers));
    if (tmp == NULL) {
        wpa_printf(MSG_ERROR, "eloop_register_read_sock kmrealloc failed");
        return -1;
    }

    tmp[eloop.reader_count].sock = sock;
    tmp[eloop.reader_count].eloop_data = eloop_data;
    tmp[eloop.reader_count].user_data = user_data;
    tmp[eloop.reader_count].handler = handler;
    eloop.reader_count++;
    eloop.readers = tmp;
    if (sock > eloop.max_sock)
        eloop.max_sock = sock;
    eloop.reader_table_changed = 1;

    return 0;
}


void eloop_unregister_read_sock(int sock)
{
    int i;

    if (eloop.readers == NULL || eloop.reader_count == 0)
        return;

    for (i = 0; i < eloop.reader_count; i++) {
        if (eloop.readers[i].sock == sock)
            break;
    }
    if (i == eloop.reader_count)
        return;
    if (i != eloop.reader_count - 1) {
        memmove(&eloop.readers[i], &eloop.readers[i + 1],
            (eloop.reader_count - i - 1) *
            sizeof(struct eloop_sock));
    }
    eloop.reader_count--;
    eloop.reader_table_changed = 1;
}


int eloop_sock_requeue(void)
{
    return 0;
}


int eloop_register_timeout(unsigned int secs, unsigned int usecs,
               eloop_timeout_handler handler,
               void *eloop_data, void *user_data)
{
    struct eloop_timeout *timeout, *tmp, *prev;
    wpa_printf(MSG_EXCESSIVE, "eloop_register_timeout: %u sec %u usecs", secs, usecs);

    timeout = (struct eloop_timeout *) kmalloc(sizeof(*timeout));
    if (timeout == NULL) {
        wpa_printf(MSG_ERROR, "eloop_register_timeout kmalloc failed");
        return -1;
    }
    os_get_reltime(&timeout->time);
    timeout->time.sec += secs;
    timeout->time.usec += usecs;
    while (timeout->time.usec >= 1000000) {
        timeout->time.sec++;
        timeout->time.usec -= 1000000;
    }
    timeout->eloop_data = eloop_data;
    timeout->user_data = user_data;
    timeout->handler = handler;
    timeout->next = NULL;

    if (eloop.timeout == NULL) {
        eloop.timeout = timeout;
        return 0;
    }

    prev = NULL;
    tmp = eloop.timeout;
    while (tmp != NULL) {
        if (os_reltime_before(&timeout->time, &tmp->time))
            break;
        prev = tmp;
        tmp = tmp->next;
    }

    if (prev == NULL) {
        timeout->next = eloop.timeout;
        eloop.timeout = timeout;
    } else {
        timeout->next = prev->next;
        prev->next = timeout;
    }
    wpa_printf(MSG_EXCESSIVE, "eloop_register_timeout ok");
    return 0;
}


int eloop_cancel_timeout(eloop_timeout_handler handler,
             void *eloop_data, void *user_data)
{
    struct eloop_timeout *timeout, *prev, *next;
    int removed = 0;

    prev = NULL;
    timeout = eloop.timeout;
    while (timeout != NULL) {
        next = timeout->next;

        if (timeout->handler == handler &&
            (timeout->eloop_data == eloop_data ||
             eloop_data == ELOOP_ALL_CTX) &&
            (timeout->user_data == user_data ||
             user_data == ELOOP_ALL_CTX)) {
            if (prev == NULL)
                eloop.timeout = next;
            else
                prev->next = next;
            kmfree(timeout);
            removed++;
        } else
            prev = timeout;

        timeout = next;
    }

    return removed;
}


int eloop_cancel_timeout_one(eloop_timeout_handler handler,
                 void *eloop_data, void *user_data,
                 struct os_reltime *remaining)
{
    struct eloop_timeout *timeout, *prev;
    int removed = 0;
    struct os_reltime now;

    os_get_reltime(&now);
    remaining->sec = remaining->usec = 0;

    prev = NULL;
    timeout = eloop.timeout;
    while (timeout != NULL) {
        if (timeout->handler == handler &&
            (timeout->eloop_data == eloop_data) &&
            (timeout->user_data == user_data)) {
            removed = 1;
            if (os_reltime_before(&now, &timeout->time))
                os_reltime_sub(&timeout->time, &now, remaining);
            if (prev == NULL)
                eloop.timeout = timeout->next;
            else
                prev->next = timeout->next;
            kmfree(timeout);
            break;
        }

        prev = timeout;
        timeout = timeout->next;
    }

    return removed;
}


int eloop_deplete_timeout(unsigned int req_secs, unsigned int req_usecs,
              eloop_timeout_handler handler, void *eloop_data,
              void *user_data)
{
    struct os_reltime now, requested, remaining;
    struct eloop_timeout *tmp;

    tmp = eloop.timeout;
    while (tmp != NULL) {
        if (tmp->handler == handler &&
            tmp->eloop_data == eloop_data &&
            tmp->user_data == user_data) {
            requested.sec = req_secs;
            requested.usec = req_usecs;
            os_get_reltime(&now);
            os_reltime_sub(&tmp->time, &now, &remaining);
            if (os_reltime_before(&requested, &remaining)) {
                eloop_cancel_timeout(handler, eloop_data,
                             user_data);
                eloop_register_timeout(requested.sec,
                               requested.usec,
                               handler, eloop_data,
                               user_data);
                return 1;
            }
            return 0;
        }

        tmp = tmp->next;
    }

    return -1;
}


int eloop_is_timeout_registered(eloop_timeout_handler handler,
                void *eloop_data, void *user_data)
{
    struct eloop_timeout *tmp;

    tmp = eloop.timeout;
    while (tmp != NULL) {
        if (tmp->handler == handler &&
            tmp->eloop_data == eloop_data &&
            tmp->user_data == user_data)
            return 1;

        tmp = tmp->next;
    }

    return 0;
}


int eloop_register_signal_terminate(eloop_signal_handler handler,
                    void *user_data)
{
    return 0;
}


int eloop_register_signal_reconfig(eloop_signal_handler handler,
                   void *user_data)
{
    return 0;
}


void eloop_run(void)
{
    int i;
    volatile struct os_reltime now;

    //wpa_printf(MSG_EXCESSIVE, "eloop_run start");

    while (!eloop.terminate) {
        // 0 sec, 10000/200000 usec delay
        os_sleep(0, l2_packet_auth_active () ? 10000 : 200000);

        /* 登録されているtimeoutが発火するかチェックする */
        if (eloop.timeout) {
            struct eloop_timeout *tmp;

            os_get_reltime(&now);
            if (!os_reltime_before(&now, &eloop.timeout->time)) {
                //wpa_printf(MSG_EXCESSIVE, "fire %u.%u",
                //    eloop.timeout->time.sec, eloop.timeout->time.usec);
                tmp = eloop.timeout;
                eloop.timeout = eloop.timeout->next;
                tmp->handler(tmp->eloop_data, tmp->user_data);
                kmfree(tmp);
            }
        }

        eloop.reader_table_changed = 0;
        for (i = 0; i < eloop.reader_count; i++) {
            eloop.readers[i].handler(
                eloop.readers[i].sock,
                eloop.readers[i].eloop_data,
                eloop.readers[i].user_data);
            if (eloop.reader_table_changed)
                break;
        }
    }
    //wpa_printf(MSG_EXCESSIVE, "eloop_run end");
}


void eloop_terminate(void)
{
    eloop.terminate = 1;
}


void eloop_destroy(void)
{
    struct eloop_timeout *timeout, *prev;

    timeout = eloop.timeout;
    while (timeout != NULL) {
        prev = timeout;
        timeout = timeout->next;
        kmfree(prev);
    }
    kmfree(eloop.readers);
}
