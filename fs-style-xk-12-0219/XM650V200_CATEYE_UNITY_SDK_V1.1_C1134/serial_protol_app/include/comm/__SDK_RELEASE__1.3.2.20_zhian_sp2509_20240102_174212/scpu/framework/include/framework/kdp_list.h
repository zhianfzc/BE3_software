#ifndef __KDP_LIST_H__
#define __KDP_LIST_H__


#include "types.h"
//#include "framework/framework_types.h"

struct kdp_list {
    struct kdp_list *next, *prev;
};

struct kdp_list_node {
    struct kdp_list list;
};

struct kdp_list_iter {
    struct kdp_list *i_list;
    struct kdp_list_node *i_cur;
};

#define kdp_list_first_entry(ptr, type, member) \
    container_of((ptr)->next, type, member)

#define kdp_list_next_entry(pos, member) \
    container_of((pos)->member.next, typeof(*(pos)), member)


void kdp_list_init(struct kdp_list *list);
void kdp_list_add_tail(struct kdp_list *head, struct kdp_list *new_list);
void kdp_list_add_node_tail(struct kdp_list *list, struct kdp_list_node *node);
void kdp_list_del(struct kdp_list *list);
void kdp_list_del_init(struct kdp_list *list);
int  kdp_list_empty(const struct kdp_list *list);

void kdp_list_iter_init_node(struct kdp_list *list, struct kdp_list_iter *iter);
void kdp_list_iter_exit(struct kdp_list_iter *iter);
struct kdp_list_node *kdp_list_next(struct kdp_list_iter *iter);


#endif
