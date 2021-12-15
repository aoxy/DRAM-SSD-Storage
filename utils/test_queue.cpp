#include <iostream>
#include "xhqueue.h"
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
        std::cout << queue;
        std::cout << queue.size() << std::endl;
    }

    return 0;
}