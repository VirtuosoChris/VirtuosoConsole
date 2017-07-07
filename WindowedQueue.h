//
//  RingBuffer.h
//  VirtuosoConsole
//
//  Created by Chris Pugh on 7/7/17.
//
//

#ifndef RingBuffer_h
#define RingBuffer_h

#include <deque>
namespace Virtuoso
{
    const std::size_t defaultRingbufferCapacity=10;

    template <typename T>
    class WindowedQueue
    {
    private:
        std::deque<T> data;

        void shrinkToCapacity()
        {
            while (data.size() > capacity)
            {
                data.pop_front();
            }
        }

    public:

        std::size_t capacity;

        WindowedQueue(std::size_t inputCapacity = defaultRingbufferCapacity) : capacity(inputCapacity)
        {
        }

        const std::string& operator[](std::size_t idx)
        {
            return data[idx];
        }

        void append(const T& str)
        {
            data.push_back(str);
            shrinkToCapacity();
        }

        std::size_t size() const
        {
            return data.size();
        }
    };
}
#endif /* RingBuffer_h */
