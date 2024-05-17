/*
 * @name : kdp_list.c
 * @brief : Library for manipulating klists.
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "framework/kdp_list.h"
#include "framework/utils.h"
#include <stddef.h>
#include "dbg.h"


static void _list_reset(struct kdp_list *list) {
    list->next = list;
    list->prev = list;
}

static void _list_add(struct kdp_list *new_list,
        struct kdp_list *prev,
        struct kdp_list *next)
{
    next->prev = new_list;
    new_list->next = next;
    new_list->prev = prev;

    if (prev)
        prev->next = new_list;
}

static inline void _list_del(struct kdp_list *prev, struct kdp_list *next)
{
    next->prev = prev;
    prev->next = next;
}

static void _klist_node_init(struct kdp_list *list, struct kdp_list_node *node)
{
    _list_reset(&node->list);
}

static struct kdp_list_node *_to_klist_node(struct kdp_list *n)
{
    return container_of(n, struct kdp_list_node, list);
}


void kdp_list_init(struct kdp_list *list)
{
    _list_reset(list);
}

void kdp_list_add_tail(struct kdp_list *head, struct kdp_list *new_list)
{
    _list_add(new_list, head->prev, head);
}

void kdp_list_add_node_tail(struct kdp_list *list, struct kdp_list_node *node)
{
    _klist_node_init(list, node);
    kdp_list_add_tail(list, &node->list);
}

void kdp_list_del(struct kdp_list *list)
{
    _list_del(list->prev, list->next);

    list->next = NULL;
    list->prev = NULL;
}

void kdp_list_del_init(struct kdp_list *list)
{
    _list_del(list->prev, list->next);
    _list_reset(list);    

}

int kdp_list_empty(const struct kdp_list *head)
{
    return ((head->next) == head);
}

void kdp_list_iter_init_node(struct kdp_list *list, struct kdp_list_iter *i)//,struct kdp_list_node *node)
{
    i->i_list = list;
    i->i_cur = NULL;
    //if (node)
    //    i->i_cur = node;
}

void kdp_list_iter_exit(struct kdp_list_iter *iter)
{
    if (iter->i_cur) {
        iter->i_cur = NULL;
    }
}

struct kdp_list_node *kdp_list_next(struct kdp_list_iter *iter)
{
    struct kdp_list_node *last = iter->i_cur;
    struct kdp_list_node *next;

    if (last) {
        next = _to_klist_node(last->list.next);
    } else
        next = _to_klist_node(iter->i_list->next);
    iter->i_cur = NULL;
    while (next != _to_klist_node(iter->i_list)) {
        iter->i_cur = next;
        break;
    }
    return iter->i_cur;
}
