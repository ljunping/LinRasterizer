//
// Created by Lin on 2024/11/15.
//

#ifndef LINKLIST_H
#define LINKLIST_H
#include "PODPool.h"

template <class T>
struct  LinkNode
{
    T data;
    LinkNode<T>* next;
    LinkNode<T>* prev;
};

template <class T>
class LinkList
{
public:
    LinkNode<T>* head;
    LinkNode<T>* tail;
    int count=0;
    LinkList()
    {
        head = PODPool<LinkNode<T>>::POOL.create();
    }
    ~LinkList()
    {
        while (head!= nullptr)
        {
            LinkNode<T>* temp = head->next;
            head->next = nullptr;
            head->data=nullptr;
            head->prev = nullptr;
            PODPool<LinkNode<T>>::POOL.recycle(head);
            head = temp;
        }
    }
    LinkNode<T>* add_first(T data)
    {
        LinkNode<T>* node = PODPool<LinkNode<T>>::POOL.create();
        node->data = data;
        auto next = head->next;
        head->next = node;
        if (next) next->prev = node;
        node->next = next;
        node->prev = head;
        if (!node->next)
        {
            tail = node;
        }
        count++;
        return node;
    }
    LinkNode<T>* add_last(T data)
    {
        LinkNode<T>* node = PODPool<LinkNode<T>>::POOL.create();
        node->data = data;
        tail->next = node;
        node->prev = tail;
        tail = node;
        count++;
        return node;
    }
    void remove(LinkNode<T>* node)
    {
        if (tail == node)
        {
            tail = nullptr;
        }
        if (node->prev)
        {
            node->prev->next = node->next;
            if (!tail)
            {
                tail = node->prev;
            }
        }
        if (node->next)
        {
            node->next->prev = node->prev;
        }
        node->next = nullptr;
        node->data=nullptr;
        node->prev = nullptr;
        PODPool<LinkNode<T>>::POOL.recycle(node);
        count--;
    }

    void clear()
    {
        while (head!= nullptr)
        {
            LinkNode<T>* temp = head->next;
            head->next = nullptr;
            head->data=nullptr;
            head->prev = nullptr;
            PODPool<LinkNode<T>>::POOL.recycle(head);
            head = temp;
        }
        head = PODPool<LinkNode<T>>::POOL.create();
        count = 0;
    }
};


#endif //LINKLIST_H
