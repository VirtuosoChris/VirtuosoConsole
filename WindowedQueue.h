//
//  WindowedQueue.h
//  VirtuosoConsole
//
//  Created by Steve Braeger and Chris Pugh on 7/7/17.
//
//

#ifndef RingBuffer_h
#define RingBuffer_h

#include <queue>

namespace Virtuoso
{
    // wtf ? --> http://en.cppreference.com/w/cpp/language/using_declaration#Inheriting_constructors
    template<class T>
    class WindowedQueue: protected std::queue<T>
    {
        void fix_size()
        {
            while (size() > m_capacity)
            {
                std::queue<T>::pop();
            }
        }

        std::size_t m_capacity;

    public:

        WindowedQueue(std::size_t maxCapacity) : m_capacity(maxCapacity)
        {
        }

        void capacity(std::size_t newCapacity)
        {
            m_capacity = newCapacity;
            fix_size();
        }

        std::size_t capacity() const
        {
            return m_capacity;
        }

        ///using std::queue<T>::queue;
        using std::queue<T>::operator=;
        using std::queue<T>::front;
        using std::queue<T>::back;
        using std::queue<T>::empty;
        using std::queue<T>::size;
        using std::queue<T>::pop;
        using std::queue<T>::swap;

        T& operator[](size_t idx)
        {
            return this->c[idx];
        }

        const T& operator[](size_t idx) const
        {
            return this->c[idx];
        }

        template<class... Args>
        void emplace(Args&&... args)
        {
            std::queue<T>::emplace(std::forward<Args>(args)...);
            fix_size();
        }

        void push( const T& value )
        {
            std::queue<T>::push(value);
            fix_size();
        }
        
        void push( T&& value )
        {
            std::queue<T>::push(value);
            fix_size();
        }      
    };
}
#endif /* RingBuffer_h */
