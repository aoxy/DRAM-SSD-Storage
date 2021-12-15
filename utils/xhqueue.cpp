/*
#include "xhqueue.h"

template <class T>
xhqueue<T>::xhqueue(size_t max_size) : begin(0), end(0), capacity(max_size + 1)
{
    elements = new T[capacity];
    assert(elements != nullptr);
}

template <class T>
bool xhqueue<T>::push(const T &x)
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

template <class T>
std::ostream &operator<<(std::ostream &os, xhqueue<T> &q)
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

/*
int main()
{
    size_t i;
    size_t j;
    xhqueue<size_t> queue(10);
    queue.push(3); // 队列插入元素
    queue.push(5);
    queue.push(10);
    for (int i = 12; i < 123; i++)
    {
        queue.push(i);
        cout << queue;
        cout << queue.size() << endl;
    }

    return 0;
}
*/