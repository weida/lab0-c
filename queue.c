#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *node = NULL, *tmp = NULL;
    list_for_each_entry_safe(node, tmp, head, list) {
        q_release_element(node);
    }
    free(head);
    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;

    size_t size = strlen(s) + 1;
    node->value = malloc(sizeof(char) * size);
    if (!node->value) {
        q_release_element(node);
        return false;
    }
    strncpy(node->value, s, size - 1);
    node->value[size - 1] = 0;
    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_first_entry(head, element_t, list);
    list_del(&node->list);
    if (sp && node->value && bufsize) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = 0;
    }
    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int size = 0;
    element_t *node;
    list_for_each_entry(node, head, list) {
        size++;
    }
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head *slow = head->next, *fast = head->next;

    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    element_t *node = list_entry(slow, element_t, list);
    list_del(slow);
    q_release_element(node);

    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head *cur, *next;

    list_for_each_safe(cur, next, head) {
        element_t *cur_node = list_entry(cur, element_t, list);
        bool del_cur = false;
        /* Remove all nodes with same value as the cur_node*/
        while (next != head) {
            element_t *next_node = list_entry(cur->next, element_t, list);
            if (strcmp(cur_node->value, next_node->value)) {
                break;
            }
            list_del(next);
            q_release_element(next_node);
            next = cur->next;
            del_cur = true;
        }
        /*Delete cur_node if it is duplicated*/
        if (del_cur) {
            list_del(cur);
            q_release_element(cur_node);
        }
    }
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return;
    }

    for (struct list_head **node = &(head->next);
         *node != head && (*node)->next != head; node = &(*node)->next->next) {
        struct list_head *tmp = *node;
        // second node
        *node = (*node)->next;
        // first node next point to  third node
        tmp->next = (*node)->next;
        // second node next point to first node
        (*node)->next = tmp;

        // second node prev point to first node prev
        (*node)->prev = tmp->prev;
        // first node prev update next pointer
        (*node)->prev->next = (*node);
        // first node prev point to second node prev
        tmp->prev = (*node);
        // second node next node update prev pointer
        tmp->next->prev = tmp;
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return;
    }

    struct list_head *cur = head;
    do {
        struct list_head *next = cur->next;
        cur->next = cur->prev;
        cur->prev = next;
        cur = next;
    } while (cur != head);
}
/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head)) {
        return;
    }

    struct list_head *cur = head->next, *start = NULL, *tail = head;
    ;
    int n = 0;
    while (cur != head) {
        n++;
        if (n == 1) {
            start = cur;
        }
        struct list_head *next = cur->next;
        if (n == k) {
            struct list_head *end = cur;
            struct list_head *prev = end->next, *node = start;

            while (node != end) {
                struct list_head *tmp = node->next;
                node->next = prev;
                node->prev = tmp;
                prev = node;
                node = tmp;
            }
            node->next = prev;
            node->prev = tail;
            tail->next = node;
            tail = start;
            n = 0;
        }
        cur = next;
    }

    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

void __merge(struct list_head *head, struct list_head *new_head, bool descend)
{
    if (!head || !new_head)
        return;

    LIST_HEAD(tmp_head);
    while (!list_empty(head) && !list_empty(new_head)) {
        element_t *first_node = list_first_entry(head, element_t, list);
        element_t *second_node = list_first_entry(new_head, element_t, list);
        int cmp = strcmp(first_node->value, second_node->value);
        if ((descend && cmp <= 0) || (!descend && cmp >= 0))
            list_move_tail(&second_node->list, &tmp_head);
        else
            list_move_tail(&first_node->list, &tmp_head);
    }
    list_splice(&tmp_head, head);
    list_splice_tail_init(new_head, head);
    return;
}
/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    LIST_HEAD(new_head);
    list_cut_position(&new_head, head, slow->prev);

    q_sort(&new_head, descend);
    q_sort(head, descend);

    __merge(head, &new_head, descend);
    return;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int n = 1;
    element_t *cur = list_last_entry(head, element_t, list);
    while (cur->list.prev != head) {
        element_t *prev = list_last_entry(&cur->list, element_t, list);
        if (strcmp(prev->value, cur->value) > 0) {
            list_del(&prev->list);
            q_release_element(prev);
        } else {
            n++;
            cur = prev;
        }
    }

    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return n;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int n = 1;
    element_t *cur = list_last_entry(head, element_t, list);
    while (cur->list.prev != head) {
        element_t *prev = list_last_entry(&cur->list, element_t, list);
        if (strcmp(prev->value, cur->value) < 0) {
            list_del(&prev->list);
            q_release_element(prev);
        } else {
            n++;
            cur = prev;
        }
    }

    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return n;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    queue_contex_t *first = list_first_entry(head, queue_contex_t, chain);
    int result = q_size(first->q);

    if (list_is_singular(head))
        return result;

    queue_contex_t *second =
        list_entry(first->chain.next, queue_contex_t, chain);

    while (&second->chain != head) {
        result += q_size(second->q);
        __merge(first->q, second->q, descend);
        second = list_entry(second->chain.next, queue_contex_t, chain);
    }
    return result;
}
