/**
 **************************************************************************************
 * @file    llist.h
 * @brief
 *
 * @author  Castle Cai
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#ifndef _LLIST_H
#define _LLIST_H

/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})


/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized llist entries.
 */
#define LLIST_POISON1  (0)
#define LLIST_POISON2  (0)

/*
 * Simple doubly linked llist implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole llists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

typedef struct llist_head_t
{
	struct llist_head_t *next, *prev;
} llist_head;

#define LLIST_HEAD_INIT(name) { &(name), &(name) }

#define LLIST_HEAD(name) \
	llist_head name = LLIST_HEAD_INIT(name)

#define INIT_LLIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal llist manipulation where we know
 * the prev/next entries already!
 */
static void __llist_add(llist_head *p,
                        llist_head *prev,
                        llist_head *next)
{
	next->prev = p;
	p->next = next;
	p->prev = prev;
	prev->next = p;
}

/**
 * llist_add - add a new entry
 * @new: new entry to be added
 * @head: llist head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static void llist_add(llist_head *p, llist_head *head)
{
	__llist_add(p, head, head->next);
}

/**
 * llist_add_tail - add a new entry
 * @new: new entry to be added
 * @head: llist head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static void llist_add_tail(llist_head *p, llist_head *head)
{
	__llist_add(p, head->prev, head);
}

/*
 * Delete a llist entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal llist manipulation where we know
 * the prev/next entries already!
 */
static void __llist_del(llist_head * prev, llist_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * llist_del - deletes entry from llist.
 * @entry: the element to delete from the llist.
 * Note: llist_empty on entry does not return true after this, the entry is
 * in an undefined state.
 */
static void llist_del(llist_head *entry)
{
	__llist_del(entry->prev, entry->next);
	entry->next = LLIST_POISON1;
	entry->prev = LLIST_POISON2;
}

/**
 * llist_del_init - deletes entry from llist and reinitialize it.
 * @entry: the element to delete from the llist.
 */
static void llist_del_init(llist_head *entry)
{
	__llist_del(entry->prev, entry->next);
	INIT_LLIST_HEAD(entry);
}

/**
 * llist_move - delete from one llist and add as another's head
 * @llist: the entry to move
 * @head: the head that will precede our entry
 */
static void llist_move(llist_head *llist, llist_head *head)
{
	__llist_del(llist->prev, llist->next);
	llist_add(llist, head);
}

/**
 * llist_move_tail - delete from one llist and add as another's tail
 * @llist: the entry to move
 * @head: the head that will follow our entry
 */
static void llist_move_tail(llist_head *llist, llist_head *head)
{
	__llist_del(llist->prev, llist->next);
	llist_add_tail(llist, head);
}

/**
 * llist_empty - tests whether a llist is empty
 * @head: the llist to test.
 */
static int llist_empty(const llist_head *head)
{
	return head->next == head;
}

/**
 * llist_entry - get the struct for this entry
 * @ptr:	the &llist_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the llist_struct within the struct.
 */
#define llist_entry(ptr, type, member) \
	container_of(ptr, type, member)

#endif

