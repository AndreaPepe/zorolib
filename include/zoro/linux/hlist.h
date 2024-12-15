/**
 * @file linux/hlist.h
 * @author Andrea Pepe
 * @copyright Copyright (c) 2024
 *
 * @brief Doubly linked list implementation with a single pointer list head
 * (a.k.a. "hash list"). 
 *
 * This data strcuture is mostly useful for hash tables, where having two
 * pointer list head is too wasteful.
 * You lose the ability to access the tail of the list in O(1).
 *
 * Extracted from include/linux/list.h in Linux kernel 5.16.11
 */

#ifndef __ZORO_LINUX_HLIST_H__
#define __ZORO_LINUX_HLIST_H__

#include <zoro/linux/list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT \
        { .first = NULL }

#define HLIST_HEAD(name) \
        struct hlist_head name = {  .first = NULL }

#define INIT_HLIST_HEAD(ptr) \
        ((ptr)->first = NULL)

/**
 * @brief Initialize a @a struct @a hlist_node.  
 *
 * @param h             Pointer to the @a struct @a hlist_node to initialize.
 */
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

/**
 * @brief Test whether a node has been removed from list and reinitialized.
 *
 * @note Not all removal functions will leave a node in unhashed state. For
 * example, @a hlist_nulls_del_init_rcu() does leave the node in unhashed
 * state, but @a hlist_nulls_del() does not.
 *
 * @param h             the node to be checked
 *
 * @return Return 1 if @a h has been reinitialized; 0 otherwise. 
 */
static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

/**
 * @brief Verrsion of @a hlist_unhashed for lockless use.
 *
 * This variant of @a hlist_unhashed() must be used in lockless contexts
 * to avoid potential load-tearing.  The @a READ_ONCE() is paired with the
 * various @a WRITE_ONCE() in hlist helpers that are defined below.
 * 
 * @param h             the node to be checked
 *
 * @return Return 1 if @a h has been reinitialized; 0 otherwise. 
 */
static inline int hlist_unhashed_lockless(const struct hlist_node *h)
{
	return !READ_ONCE(h->pprev);
}

/**
 * @brief Test whether the specified hlist_head structure is an empty hlist.
 *
 * @param h             structure to check
 *
 * @return Return 1 if empty; 0 otherwise.
 */
static inline int hlist_empty(const struct hlist_head *h)
{
	return !READ_ONCE(h->first);
}

/**
 * @brief Delete a note from the list.
 *
 * @warning Internal use only!!!
 *
 * @param n             node to delete
 */
static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;

	WRITE_ONCE(*pprev, next);
	if (next)
		WRITE_ONCE(next->pprev, pprev);
}

/**
 * @brief Delete the specified hlist_node from its list.
 * 
 * @note This function leaves the node in hashed state. Use @a hlist_del_init() 
 * or similar instead to unhash @a n.
 *
 * @param n             node to delete
 */
static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = (struct hlist_node *)LIST_POISON1;
	n->pprev = (struct hlist_node **)LIST_POISON2;
}

/**
 * @brief Delete the specified hlist_node from its list and initialize it.
 *
 * @note This function leaves the node in unhashed state.
 *
 * @param n             node to delete
 */
static inline void hlist_del_init(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

/**
 * @brief Add a new entry at the beginning of the hlist.
 * 
 * Insert a new entry after the specified head @a h.
 * This is good for implementing stacks.
 *
 * @param n             new entry to be added
 * @param h             hlist_head to add it after
 */
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	WRITE_ONCE(n->next, first);
	if (first)
		WRITE_ONCE(first->pprev, &n->next);
	WRITE_ONCE(h->first, n);
	WRITE_ONCE(n->pprev, &h->first);
}

/**
 * @brief Add a new entry before the one specified.
 * 
 * @param n             new entry to be added
 * @param next          hlist_node to add it before, which must be non-NULL
 */
static inline void hlist_add_before(struct hlist_node *n,
				    struct hlist_node *next)
{
	WRITE_ONCE(n->pprev, next->pprev);
	WRITE_ONCE(n->next, next);
	WRITE_ONCE(next->pprev, &n->next);
	WRITE_ONCE(*(n->pprev), n);
}

/**
 * @brief Add a new entry after the one specified.
 *
 * @param n             new entry to be added
 * @param prev          hlist_node to add it after, which must be non-NULL
 */
static inline void hlist_add_behind(struct hlist_node *n,
				    struct hlist_node *prev)
{
	WRITE_ONCE(n->next, prev->next);
	WRITE_ONCE(prev->next, n);
	WRITE_ONCE(n->pprev, &prev->next);

	if (n->next)
		WRITE_ONCE(n->next->pprev, &n->next);
}

/**
 * @brief Create a fake hlist consisting of a single headless node.
 * 
 * This makes @a n appear to be its own predecessor on a headless hlist.
 * The point of this is to allow things like @a hlist_del() to work correctly
 * in cases where there is no list.
 *
 * @param n             node to make a fake list out of
 */
static inline void hlist_add_fake(struct hlist_node *n)
{
	n->pprev = &n->next;
}

/**
 * @brief Test whether a node is a fake hlist.
 *
 * @param h             node to check for being a self-referential fake hlist
 *
 * @return Return 1 if the node is a fake hlist; 0 otherwise.
 */
static inline bool hlist_fake(struct hlist_node *h)
{
	return h->pprev == &h->next;
}

/**
 * @brief Test whether a node is the only element of the specified hlist.
 *
 * Check whether the node @n is the only node of the head @a h without
 * accessing head, thus avoiding unnecessary cache misses.
 *
 * @param n             node to check for singularity
 * @param h             header for potentially singular list
 *
 */
static inline bool hlist_is_singular_node(struct hlist_node *n,
                                          struct hlist_head *h)
{
	return !n->next && n->pprev == &h->first;
}

/**
 * @brief Move an hlist.
 *
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 *
 * @param old           hlist_head for old list
 * @param _new          hlist_head for new list
 */
static inline void hlist_move_list(struct hlist_head *old,
				   struct hlist_head *_new)
{
	_new->first = old->first;
	if (_new->first)
		_new->first->pprev = &_new->first;
	old->first = NULL;
}

/** 
 * @defgroup hlist_iterators Hash list iterator macros.
 * @brief Helper macros to iterate on hlists and recover entries.
 * @{
 */

/**
 * @brief Get the struct from this entry - alias to @a container_of.
 *
 * @param ptr           pointer to @a hlist_node inside the structure
 * @param type          the type of the containing structure
 * @param member        the name of @a ptr inside the structure
 *
 * @return Return a pointer to the container structure.
 */
#define hlist_entry(ptr, type, member) \
        container_of(ptr,type,member)

/**
 * @brief Iterate over a hlist.
 * @param pos           the &struct hlist_node to use as a loop cursor
 * @param head	        the head of the list (struct hlist_head)
 */
#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

/**
 * @brief Iterate over a hlist safe against removal of entries.
 * @param pos           the &struct hlist_node to use as a loop cursor
 * @param n             another &struct hlist_node to use as temporary storage
 * @param head	        the head of the list (struct hlist_head)
 */
#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

/**
 * @brief Safely get the container struct from this entry.
 *
 * It is safe since it checks if the pointer is NULL.
 *
 * @param ptr           pointer to @a hlist_node inside the structure
 * @param type          the type of the containing structure
 * @param member        the name of @a ptr inside the structure
 *
 * @return Return a pointer to the container structure or NULL if @a ptr is
 * NULL.
 */
#define hlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
	})

/**
 * @brief Iterate over hlist of given type.
 * @param pos	        the type * to use as a loop cursor
 * @param head	        the head for your list
 * @param member	the name of the hlist_node within the struct
 */
#define hlist_for_each_entry(pos, head, member)				\
	for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member);\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * @brief Iterate over a hlist continuing after current point.
 * @param pos	        the type * to use as a loop cursor
 * @param member	the name of the hlist_node within the struct
 */
#define hlist_for_each_entry_continue(pos, member)			\
	for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member);\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * @brief Iterate over a hlist continuing from current point.
 * @param pos	        the type * to use as a loop cursor
 * @param member	the name of the hlist_node within the struct
 */
#define hlist_for_each_entry_from(pos, member)				\
	for (; pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * @brief Iterate over list of given type safe against removal of list entry.
 * @param pos	        the type * to use as a loop cursor
 * @param n		a &struct hlist_node to use as temporary storage
 * @param head	        the head for your list
 * @param member	the name of the hlist_node within the struct
 */
#define hlist_for_each_entry_safe(pos, n, head, member) 		\
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member);\
	     pos && ({ n = pos->member.next; 1; });			\
	     pos = hlist_entry_safe(n, typeof(*pos), member))
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* __ZORO_LINUX_HLIST_H__ */
