#pragma once
#include <libcxx/cassert.h>
#include <libcxx/malloc.h>
#include <libcxx/memory.h>

namespace Algo {

template <class T>
class LinkedList;

template <class T>
class LinkedListIterator {
public:
    bool operator!=(const LinkedListIterator& other) const { return m_node != other.m_node; }
    bool operator==(const LinkedListIterator& other) const { return m_node == other.m_node; }
    LinkedListIterator& operator++()
    {
        m_node = m_node->next();
        return *this;
    }
    T& operator*() { return *m_node; }
    T* operator->() { return m_node; }
    inline bool is_end() const { return !m_node; }
    static LinkedListIterator null() { return LinkedListIterator(nullptr); }

private:
    friend LinkedList<T>;
    explicit LinkedListIterator(T* node)
        : m_node(node)
    {
    }
    T* m_node;
};

template <class T>
class LinkedListNode {
public:
    LinkedListNode() = default;

    inline T* next() const { return m_next; }
    inline T* prev() const { return m_prev; }
    inline void set_prev(T* prev) { m_prev = prev; };
    inline void set_next(T* next) { m_next = next; };

private:
    T* m_prev { nullptr };
    T* m_next { nullptr };
};

template <class T>
class LinkedList {
public:
    LinkedList() { }

    bool is_empty() const { return !m_head; }

    T* head() const { return m_head; }
    T* tail() const { return m_tail; }

    void push_front(T* node);
    void push_back(T* node);
    void remove(T* node);

    using Iterator = LinkedListIterator<T>;
    friend Iterator;
    Iterator begin() { return Iterator(m_head); }
    Iterator end() { return Iterator::null(); }

private:
    T* m_head { nullptr };
    T* m_tail { nullptr };
};

template <typename T>
void LinkedList<T>::push_front(T* node)
{
    if (!m_head) {
        ASSERT(!m_tail);
        m_head = node;
        m_tail = node;
        node->set_prev(0);
        node->set_next(0);
        return;
    }

    ASSERT(m_tail);
    m_head->set_prev(node);
    node->set_next(m_head);
    node->set_prev(0);
    m_head = node;
}

template <typename T>
void LinkedList<T>::push_back(T* node)
{
    if (!m_tail) {
        ASSERT(!m_head);
        m_head = node;
        m_tail = node;
        node->set_prev(nullptr);
        node->set_next(nullptr);
        return;
    }

    ASSERT(m_head);
    m_tail->set_next(node);
    node->set_prev(m_tail);
    node->set_next(nullptr);
    m_tail = node;
}

template <typename T>
inline void LinkedList<T>::remove(T* node)
{
    if (node->prev()) {
        ASSERT(node != m_head);
        node->prev()->set_next(node->next());
    } else {
        ASSERT(node == m_head);
        m_head = node->next();
    }

    if (node->next()) {
        ASSERT(node != m_tail);
        node->next()->set_prev(node->prev());
    } else {
        ASSERT(node == m_tail);
        m_tail = node->prev();
    }
}

}

using Algo::LinkedList;
using Algo::LinkedListNode;