/**
 * @file linux/list.h
 * @author Andrea Pepe
 * @copyright Copyright (c) 2024
 *
 * @brief Circular doubly linked list implementation.
 *
 * Extracted from include/linux/list.h in Linux kernel 5.16.11
 */

#ifndef __ZORO_LINUX_LIST_H__
#define __ZORO_LINUX_LIST_H__

#include <linux/types.h>
#include <zoro/compiler.h>
#include <zoro/linux/rwonce.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef container_of

#ifndef offset_of
#define offset_of(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * @brief Cast a member of a struct out to the containing struct
 *
 * @param ptr           the pointer to the member
 * @param type          the type of the container struct the member is 
 *                      embedded in
 * @param member        the name of the member within the struct
 *
 * @return              A pointer to the container struct
 */
#define container_of(ptr, type, member) ({                      \
	const typeof(((type *)0)->member)*__mptr = (ptr);       \
	(type *)((char *)__mptr - offsetof(type, member)); })
#endif

/* Extracted from include/linux/poison.h in kernel 5.16.11 */
#define POISON_POINTER_DELTA 0
/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define LIST_POISON1  ((void *) (0x100 + POISON_POINTER_DELTA))
#define LIST_POISON2  ((void *) (0x122 + POISON_POINTER_DELTA))


/* ############################################################################
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 * ############################################################################
 */

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) \
        struct list_head name = LIST_HEAD_INIT(name)

/**
 * @brief Initialize a @a list_head structure.
 *
 * Initialize a @a list_head to point to itself. If it is a list header, the
 * result is an empty list.
 *
 * @param list          list_head structure to be initialized       
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
        WRITE_ONCE(list->next, list);
        list->prev = list;
}

/**
 * @brief Add a new entry between two consecutive entries.
 *
 * Be aware this is only for INTERNAL list manipulation where we know the
 * next/prev entries already!
 *
 * @param _new          the new entry to be added 
 * @param prev          the previous entry
 * @param next          the next entry
 */
static inline void __list_add(struct list_head *_new,
                              struct list_head *prev,
                              struct list_head *next)
{
        next->prev = _new;
        _new->next = next;
        _new->prev = prev;
        WRITE_ONCE(prev->next, _new);
}

/**
 * @brief Add a new entry after the specified head.
 *
 * This is good for implementing stacks.
 *
 * @param _new          the new entry to be added
 * @param head          @a list_head to add it after 
 */
static inline void list_add(struct list_head *_new, struct list_head *head)
{
        __list_add(_new, head, head->next);
}

/**
 * @brief Add a new entry before the specified head.
 *
 * This is good for implementing queues.
 *
 * @param _new          the new entry to be added
 * @param head          @a list_head to add it before 
 */
static inline void list_add_tail(struct list_head *_new, struct list_head *head)
{
        __list_add(_new, head->prev, head);
}

/**
 * @brief Delete a list entry my making the orev/next entries point to each
 * other.
 *
 * Be aware this is only for INTERNAL list manipulation where we know the
 * next/prev entries already!
 *
 * @param prev          the entry that preceeds the one to be deleted
 * @param next          the entry that follows the one to be deleted
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
        next->prev = prev;
        WRITE_ONCE(prev->next, next);
}

/**
 * @brief Delete a list entry and clear the @a prev pointer.
 *
 * This is a special-purpose list clearing method used in the networking code
 * for lists allocated as per-cpu, where we don't want to incur the extra
 * WRITE_ONCE() overhead of a regular @a list_del_init(). The code that uses 
 * this needs to check the node @a prev pointer instead of calling 
 * @a list_empty().
 *
 * @param entry         the entry to be deleted from the list
 */
static inline void __list_del_clearprev(struct list_head *entry)
{
        __list_del(entry->prev, entry->next);
        entry->prev = NULL;
}

/**
 * @brief Delete a list entry.
 *
 * @param entry         the entry to be deleted from the list
 */
static inline void __list_del_entry(struct list_head *entry)
{
        __list_del(entry->prev, entry->next);
}

/**
 * @brief Delete entry from list.
 *
 * @note After calling this function, @a entry is in an undefine state: @a
 * list_empty() on entry does not return true.
 *
 * @param entry         the entry to be deleted from the list 
 */
static inline void list_del(struct list_head *entry)
{
        __list_del_entry(entry);
        entry->next = (struct list_head *)LIST_POISON1;
        entry->prev = (struct list_head *)LIST_POISON2;
}


/**
 * @brief Replace a list entry with a new one.
 *
 * @param old           the element to be replaced
 * @param _new          the new element to be added
 */
static inline void list_replace(struct list_head *old,
				struct list_head *_new)
{
	_new->next = old->next;
	_new->next->prev = _new;
	_new->prev = old->prev;
	_new->prev->next = _new;
}

/**
 * @brief Replace a list entry with a new one and initialize the old one.
 *
 * If @a old was empty, it will be overwritten.
 *
 * @param old           the element to be replaced
 * @param _new          the new element to be added
 */
static inline void list_replace_init(struct list_head *old,
				     struct list_head *_new)
{
	list_replace(old, _new);
	INIT_LIST_HEAD(old);
}

/**
 * list_swap - replace entry1 with entry2 and re-add entry1 at entry2's position
 * @entry1: the location to place entry2
 * @entry2: the location to place entry1
 */
/**
 * @brief Swap two list entries.
 *
 * This function actually replaces @a entry1 with @a entry2 and then re-adds @a
 * entry1 at @a entry2's position.
 *
 * @param entry1        the location where to place @a entry 2
 * @param entry2        the location where to place @a entry 1
 */
static inline void list_swap(struct list_head *entry1,
			     struct list_head *entry2)
{
	struct list_head *pos = entry2->prev;

	list_del(entry2);
	list_replace(entry1, entry2);
	if (pos == entry1)
		pos = entry2;
	list_add(entry1, pos);
}

/**
 * @brief Delete list entry and re-initialize it.
 *
 * @param entry         the element to delete
 */
static inline void list_del_init(struct list_head *entry)
{
	__list_del_entry(entry);
	INIT_LIST_HEAD(entry);
}

/**
 * @brief Move an element from one list to another list.
 *
 * @param list          the entry to move
 * @param head          the head that will precede the entry in the new list
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del_entry(list);
	list_add(list, head);
}

/**
 * @brief Move an element from one list to the end of another list.
 *
 * @param list          the entry to move
 * @param head          the head that will follow the entry in the new list
 */
static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	__list_del_entry(list);
	list_add_tail(list, head);
}

/**
 * list_bulk_move_tail - move a subsection of a list to its tail
 * @head: the head that will follow our entry
 * @first: first entry to move
 * @last: last entry to move, can be the same as first
 *
 * Move all entries between @first and including @last before @head.
 * All three entries must belong to the same linked list.
 */
/**
 * @brief Move a subsection of a list to its tail.
 *
 * Move all entries between @a first and including @a last before @a head.
 * All three entries must belong to the same linked list.
 *
 * @param head          the head that will follow our entry
 * @param first         the first entry to move
 * @param last          the last entry to move; it can be the same as @a first
 */
static inline void list_bulk_move_tail(struct list_head *head,
				       struct list_head *first,
				       struct list_head *last)
{
	first->prev->next = last->next;
	last->next->prev = first->prev;

	head->prev->next = first;
	first->prev = head->prev;

	last->next = head;
	head->prev = last;
}

/**
 * @brief Test whether an entry is the first entry of a list.
 *
 * Check if @a list is the first entry of @a head.
 *
 * @param list          the entry to test
 * @param head          the head of the list
 *
 * @return Return 1 if @a list is the first entry of @a head; 0 otherwise.     
 */
static inline int list_is_first(const struct list_head *list,
					const struct list_head *head)
{
	return list->prev == head;
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
/**
 * @brief Test whether an entry is the last entry of a list.
 *
 * Check if @a list is the last entry of @a head.
 *
 * @param list          the entry to test
 * @param head          the head of the list 
 *
 * @return Return 1 if @a list is the last entry of @a head; 0 otherwise.
 */
static inline int list_is_last(const struct list_head *list,
				const struct list_head *head)
{
	return list->next == head;
}

/**
 * @brief Test whether a list is empty.
 *
 * @param head          the head of the list 
 *
 * @return Return 1 if @a head is an empty list; 0 otherwise. 
 */
static inline int list_empty(const struct list_head *head)
{
	return READ_ONCE(head->next) == head;
}

/**
 * @brief Delete entry from list and re-initialize it.
 *
 * This is the same as @a list_del_init(), except designed to be used together
 * with @a list_empty_careful() in a way to guarantee ordering of other memory
 * operations.
 *
 * @note Any memory operation done before a @a list_del_init_careful() is
 * guaranteed to be visible after a @a list_empty_careful() test.
 *
 * @param entry         the entry to delete from the list
 */
static inline void list_del_init_careful(struct list_head *entry)
{
	__list_del_entry(entry);
	entry->prev = entry;
	entry->next = entry;
}

/**
 * @brief Test whether a list is empty and not being modified.
 *
 * This function tests whether a list is empty and checks that no other CPU
 * might be in the process of modifying either member (@a next or @a prev).
 *
 * @note Using @a list_empty_careful() without synchronization can only be safe
 * if the only activity that can happen to the list entry is @a
 * list_del_init(). E.g. it cannot be used if another CPU could re-add it, for
 * example by calling @a list_add().
 *
 * @param head          the head of the list
 *
 * @return Return 1 if the @a head list is empty and not being modified; 0
 * otherwise. 
 */
static inline int list_empty_careful(const struct list_head *head)
{
	struct list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * @brief Rotate the list to the left.
 *
 * Move the head of the list to the tail of the list.
 *
 * @param head          the head of the list
 */
static inline void list_rotate_left(struct list_head *head)
{
	struct list_head *first;

	if (!list_empty(head)) {
		first = head->next;
		list_move_tail(first, head);
	}
}

/**
 * @brief Rotate list to specific item.
 *
 * Rotate the @a head list so that @a list becomes the new front of the list. 
 *
 * @param list          the desired new front of the list
 * @param head          the head of the list
 */
static inline void list_rotate_to_front(struct list_head *list,
					struct list_head *head)
{
	/*
	 * Deletes the list head from the list denoted by @head and
	 * places it as the tail of @list, this effectively rotates the
	 * list so that @list is at the front.
	 */
	list_move_tail(head, list);
}

/**
 * @brief Test whether a list has just one entry.
 *
 * @param head          the list to test
 *
 * @return Return 1 if @a head has only 1 entry; 0 otherwise.
 */
static inline int list_is_singular(const struct list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}

/**
 * @brief Cut a list into two.
 *
 * @param list          a new list to add all removed entries
 * @param head          the head of the list 
 * @param entry         an entry within @a head; it could be @a head itself 
 */
static inline void __list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * @brief Cut a list into two.
 *
 * Move the initial part of @a head, up to and including @a entry, from @a head
 * to @a list.
 * The @a entry element must be within @a head.
 *
 * @warning @a list should be an empty list or a list you do not care about
 * losing its data.
 *
 * @param list          a new list to add all removed entries
 * @param head          the head of the list 
 * @param entry         an entry within @a head; it could be @a head itself
 *                      and, if so, we won't cut the list 
 */
static inline void list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}

/**
 * @brief Cut a list in two, before the given entry.
 *
 * Move the initial part of @a head, up to but excluding @a entry, from @a head
 * to @a list.
 * The @a entry element must be within @a head.
 * If @a entry is equal to @a head, then all entries on @a head are moved to @a
 * list.
 *
 * @warning @a list should be an empty list or a list you do not care about
 * losing its data.
 *
 * @param list          a new list to add all removed entries
 * @param head          the head of the list 
 * @param entry         an entry within @a head; it could be @a head itself
 */
static inline void list_cut_before(struct list_head *list,
				   struct list_head *head,
				   struct list_head *entry)
{
	if (head->next == entry) {
		INIT_LIST_HEAD(list);
		return;
	}
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry->prev;
	list->prev->next = list;
	head->next = entry;
	entry->prev = head;
}

/**
 * @brief Join two lists, given the prev and the next where to link the new
 * list. 
 *
 * @param list          the new list to add 
 * @param prev          the element preceeding the new list 
 * @param next          the element following the new list  
 */
static inline void __list_splice(const struct list_head *list,
				 struct list_head *prev,
				 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * @brief Join two lists. This is designed for stacks.
 *
 * @param list          the new list to add
 * @param head          the place to add it in the first list 
 */
static inline void list_splice(const struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head, head->next);
}

/**
 * @brief Join two lists. This is designed for queues.
 *
 * @param list          the new list to add
 * @param head          the place to add it in the first list 
 */
static inline void list_splice_tail(struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head->prev, head);
}

/**
 * @brief Join two lists (stacks) and re-initialize the new list.
 *
 * The list at @a list is re-initialized.
 *
 * @param list          the new list to add
 * @param head          the place to add it in the first list 
 */
static inline void list_splice_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

/**
 * @brief Join two lists (queues) and re-initialize the new list.
 *
 * The list at @a list is re-initialized.
 *
 * @param list          the new list to add
 * @param head          the place to add it in the first list 
 */
static inline void list_splice_tail_init(struct list_head *list,
					 struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

/** 
 * @defgroup list_iterators List iterator macros.
 * @brief Helper macros to iterate on lists and recover entries.
 * @{
 */

/**
 * @brief Get the struct from this entry - alias to @a container_of
 *
 * @param ptr           pointer to @a list_head inside the structure
 * @param type          the type of the containing structure
 * @param member        the name of @a ptr inside the structure
 *
 * @return Return a pointer to the container structure.
 */
#define list_entry(ptr, type, member) \
                container_of(ptr, type, member)

/**
 * @brief Get the first element from a list.
 *
 * @warning The list is supposed to be not empty.
 *
 * @param ptr           the list head to take the element from
 * @param type          the type of the containing structure
 * @param member        the name of @a ptr inside the structure
 *
 * @return Return a pointer to the container structure.
 */
#define list_first_entry(ptr, type, member) \
                list_entry((ptr)->next, type, member)

/**
 * @brief Get the last element from a list.
 *
 * @warning The list is supposed to be not empty.
 *
 * @param ptr           the list head to take the element from
 * @param type          the type of the containing structure
 * @param member        the name of @a ptr inside the structure
 *
 * @return Return a pointer to the container structure.
 */
#define list_last_entry(ptr, type, member) \
                list_entry((ptr)->prev, type, member)


/**
 * @brief Get the first element from a list.
 *
 * @param ptr           the list head to take the element from
 * @param type          the type of the containing structure
 * @param member        the name of @a ptr inside the structure
 *
 * @return Return a pointer to the container structure or NULL if the list is
 * empty.
 */
#define list_first_entry_or_null(ptr, type, member) ({ \
	struct list_head *head__ = (ptr); \
	struct list_head *pos__ = READ_ONCE(head__->next); \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})

/**
 * @brief Get the next entry in list.
 *
 * @param pos           the type * to cursor
 * @param member        the name of the list_head within the struct
 *
 * @return Return a pointer to the container structure.
 */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * @brief Get the prev entry in list.
 *
 * @param pos           the type * to cursor
 * @param member        the name of the list_head within the struct
 *
 * @return Return a pointer to the container structure.
 */
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * @brief Iterate over a list.
 * @param pos           the &struct list_head to use as a loop cursor
 * @param head	        the head of the list
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief Continue iteration over a list.
 * 
 * Continue to iterate over a list, continuing after the current position.
 * 
 * @pos	                the &struct list_head to use as a loop cursor
 * @param head          the head of the list
 */
#define list_for_each_continue(pos, head) \
	for (pos = pos->next; pos != (head); pos = pos->next)

/**
 * @brief Iterate over a list backwards.
 * @param pos           the &struct list_head to use as a loop cursor
 * @param head	        the head of the list
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * @brief Iterate over a list safe against removal of list entries.
 * @param pos           the &struct list_head to use as a loop cursor
 * @param n             another &struct list_head to use as temporary storage
 * @param head	        the head of the list
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * @brief Iterate over a list backwards and safe against removal of list 
 * entries.
 * @param pos           the &struct list_head to use as a loop cursor
 * @param n             another &struct list_head to use as temporary storage
 * @param head	        the head of the list
 */
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     pos != (head); \
	     pos = n, n = pos->prev)

/**
 * @brief Test whether the entry points to the head of the list
 * @param pos	        the type * to cursor
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

/**
 * @brief Iterate over list of given type
 * @param pos	        the type * to use as a loop cursor
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

/**
 * @brief Iterate backwards over list of given type.
 * @param pos	        the type * to use as a loop cursor
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_last_entry(head, typeof(*pos), member);		\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = list_prev_entry(pos, member))

/**
 * @brief Prepare a pos entry for use in list_for_each_entry_continue().
 *
 * If @a pos is not already set, this helper sets it to the first entry of the
 * list.
 *
 * @param pos	        the type * to use as a start point
 * @param head	        the head of the list
 * @param member	the name of the list_head within the struct
 */
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * @brief Continue iteration over list of given type.
 * 
 * Continue to iterate over list of given type, continuing after
 * the current position.
 * 
 * @param pos	        the type * to use as a loop cursor
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 *
 */
#define list_for_each_entry_continue(pos, head, member) 		\
	for (pos = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

/**
 * @brief Iterate backwards from the given point
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 * 
 * @param pos	        the type * to use as a loop cursor
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_prev_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_prev_entry(pos, member))

/**
 * @brief Iterate over list of given type from the current point.
 *
 * Iterate over list of given type, continuing from current @a pos position.
 *
 * @param pos	        the type * to use as a loop cursor
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_from(pos, head, member) 			\
	for (; !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

/**
 * @brief Iterate backwards over list of given type from the current point.
 *
 * Iterate backwards over list of given type, continuing from current @a pos 
 * position.
 *
 * @param pos	        the type * to use as a loop cursor
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_from_reverse(pos, head, member)		\
	for (; !list_entry_is_head(pos, head, member);			\
	     pos = list_prev_entry(pos, member))

/**
 * @brief Iterate over list of given type safe against removal of list entry.
 * @param pos	        the type * to use as a loop cursor
 * @param n		another type * to use as temporary storage
 * @param head	        the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_next_entry(n, member))

/**
 * @brief Continue list iteration safe against removal.
 *
 * Iterate over list of given type, continuing after current point, safe 
 * against removal of list entry.
 *
 * @param pos	        the type * to use as a loop cursor
 * @param n		another type * to use as temporary storage
 * @param head  	the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_safe_continue(pos, n, head, member) 	\
	for (pos = list_next_entry(pos, member), 			\
		n = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member);			\
	     pos = n, n = list_next_entry(n, member))

/**
 * @brief Iterate over list from current point safe against removal.
 *
 * Iterate over list of given type from current point, safe against removal of 
 * list entry.
 *
 * @param pos	        the type * to use as a loop cursor
 * @param n		another type * to use as temporary storage
 * @param head  	the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_safe_from(pos, n, head, member) 		\
	for (n = list_next_entry(pos, member);				\
	     !list_entry_is_head(pos, head, member);			\
	     pos = n, n = list_next_entry(n, member))

/**
 * @brief Iterate backwards over list safe against removal.
 *
 * Iterate backwards over list of given type, safe against removal of list 
 * entry.
 *
 * @param pos	        the type * to use as a loop cursor
 * @param n		another type * to use as temporary storage
 * @param head  	the head for your list
 * @param member	the name of the list_head within the struct
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_last_entry(head, typeof(*pos), member),		\
		n = list_prev_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_prev_entry(n, member))

/**
 * @brief Reset a stale @a list_for_each_entry_safe loop.
 *
 * @param pos	        the loop cursor used in the @a list_for_each_entry_safe 
 *                      loop
 * @param n		temporary storage used in @a list_for_each_entry_safe
 * @param member	the name of the list_head within the struct
 *
 * @warning @a list_safe_reset_next is not safe to use in general if the list 
 * may be modified concurrently (eg. the lock is dropped in the loop body). 
 * An exception to this is if the cursor element (@a pos) is pinned in the 
 * list, and @a list_safe_reset_next is called after re-taking the lock and 
 * before completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, member)				\
	n = list_next_entry(pos, member)

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __ZORO_LINUX_LIST_H__ */
