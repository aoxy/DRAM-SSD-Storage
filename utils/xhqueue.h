#pragma once
#include <iostream>
template <class T>
class xhqueue
{
public:
    size_t end, begin;
    size_t capacity;
    T *elements;

    xhqueue(size_t max_size = 1024) : begin(0), end(0), capacity(max_size + 1)
    {
        elements = new T[capacity];
        if (elements == nullptr)
        {
            LOGINFO << "malloc failed." << std::endl;
            exit(1);
        }
        assert(elements != nullptr);
    }
    ~xhqueue() { delete[] elements; }
    bool push(const T &x)
    {
        if (full() == true)
        {
            elements[end] = x;
            end = (end + 1) % capacity;
            begin = (begin + 1) % capacity;
        }
        else
        {
            elements[end] = x;
            end = (end + 1) % capacity;
        }
        return true;
    }
    void clear() { begin = end = 0; }
    bool empty() const { return (begin == end) ? true : false; }
    bool full() const { return ((end + 1) % capacity == begin) ? true : false; }
    size_t size() const { return (end - begin + capacity) % capacity; }
    size_t next(size_t p) { return (p + 1) % capacity; }
    template <class Ty>
    friend std::ostream &operator<<(std::ostream &os, xhqueue<Ty> &q)
    {
        os << "(" << q.size() << ")\t[";
        for (size_t i = q.begin; i != q.end; i = (i + 1) % q.capacity)
        {
            if ((i + 1) % q.capacity == q.end)
            {
                os << q.elements[i];
            }
            else
            {
                os << q.elements[i] << ", ";
            }
        }
        os << "]" << std::endl;
        return os;
    }
};
