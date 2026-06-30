#include <types.h>
#include <wlan/p9ether.h>
#include <mm.h>
#include <net/util.h>


// サイズがsizeのブロックを割り当てて返す
Block *allocb(size_t size)
{
    static const size_t maxhdrsize = 64;

    size += sizeof (Block) + maxhdrsize;

    Block *b = kmalloc(size);
    assert (b != 0);

    b->buf = b->data;

    b->next = NULL;
    b->lim = b->buf + size;
    b->wp = b->buf + maxhdrsize;
    b->rp = b->wp;

    return b;
}

// ブロックbを開放する
void freeb (Block *b)
{
    kmfree(b);
}

// ブロックのrpをsizeだけ小さくする
Block *padblock (Block *b, int size)
{
    assert (size > 0);

    assert (b != 0);
    assert (b->rp - b->buf >= size);
    b->rp -= size;

    return b;
}

// キューに入っている要素の数を返す
unsigned qlen(Queue *q)
{
    return q->nelem;
}

// キューqの先頭を取り出して返す
Block *qget(Queue *q)
{
    Block *b = 0;

    assert (q != 0);
    if (q->first != 0) {
        b = q->first;

        q->first = b->next;
        if (q->first == 0) {
            assert(q->last == b);
            q->last = 0;
        }

        assert(q->nelem > 0);
        q->nelem--;
    }

    return b;
}

// キューqにブロックbを追加する
void qpass(Queue *q, Block *b)
{
    assert (b != 0);
    b->next = 0;

    assert (q != 0);
    if (q->first == 0) {
        q->first = b;
    } else {
        assert (q->last != 0);
        assert (q->last->next == 0);
        q->last->next = b;
    }
    q->last = b;
    q->nelem++;
}

// ushortの*pをリトルエンディアンに変換して返す
uint16_t nhgets(const void *p)
{
    return ntoh16(*(uint16_t *)p);
}

// uintの*pをリトルエンディアンに変換して返す
uint32_t nhgetl(const void *p)
{
    return ntoh32(*(uint32_t *)p);
}

extern int ether_addr_pton(const char *p, uint8_t *n);  // net/ether.c
// macaddr "AA:BB:CC:DD:EE:FF"の各バイトをucharに変換してaddr[6]にセットする
int parseether(uchar *addr, const char *str)
{
    return ether_addr_pton(str, addr);
}
