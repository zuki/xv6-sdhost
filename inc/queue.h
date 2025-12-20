
#ifndef _SRC_KERNEL_MISC_QUEUE_H
#define _SRC_KERNEL_MISC_QUEUE_H

#include <types.h>
#include <spinlock.h>

/* queue構造体 */
struct queue {
	struct queue_node *head;
	struct queue_node *tail;
    struct spinlock lock;
};

/* queue node構造体 */
struct queue_node {
	struct queue_node *next;
	struct queue_node *prev;
};

/* queueを初期化する */
static inline void _queue_init(struct queue *queue)
{
	queue->head = NULL;
	queue->tail = NULL;
    initlock(&queue->lock, "queue");
}

/* queue nodeを初期化する */
static inline void _queue_node_init(struct queue_node *node)
{
	node->next = NULL;
	node->prev = NULL;
}

/* qeueuの先頭にnodeを追加する */
static inline void _queue_insert(struct queue *queue, struct queue_node *node)
{
	if (queue->head)
		queue->head->prev = node;
	else
		queue->tail = node;
	node->next = queue->head;
	node->prev = NULL;
	queue->head = node;
}

/* queueのafterノードの後ろにnodeを挿入する */
static inline void _queue_insert_after(struct queue *queue, struct queue_node *node, struct queue_node *after)
{
	struct queue_node *tail;

	// If `after` is NULL then insert at the start of the list (ie. queue->head)
	if (after)
		tail = after->next;
	else
		tail = queue->head;

	// Connect the tail of the list to the node
	if (tail)
		tail->prev = node;
	else
		queue->tail = node;
	node->next = tail;

	// Connect the list up to and including `after` to the node
	if (after)
		after->next = node;
	else
		queue->head = node;
	node->prev = after;
}

/* queueからnodeを削除する */
static inline void _queue_remove(struct queue *queue, struct queue_node *node)
{
	if (node->next)
		node->next->prev = node->prev;
	else
		queue->tail = node->prev;

	if (node->prev)
		node->prev->next = node->next;
	else
		queue->head = node->next;

	node->next = NULL;
	node->prev = NULL;
}

/* queueの先頭を返す */
static inline void *_queue_head(struct queue *queue)
{
       return queue->head;
}

/* queueの末尾を返す */
static inline void *_queue_tail(struct queue *queue)
{
       return queue->tail;
}

/* queue上のnodeの次のノードを返す */
static inline void *_queue_next(struct queue_node *node)
{
       return node->next;
}

/* queue上のnodeの前のノードを返す */
static inline void *_queue_prev(struct queue_node *node)
{
       return node->prev;
}

#endif
