#pragma once

#include <atomic>
#include <iostream>
template<class T, typename... Args>
class QueueNoBlock {
public:
    QueueNoBlock() {
        _len = 0;
        _tail = nullptr;
        _front = nullptr;
    }
    ~QueueNoBlock(){}

public:
    void push(Args... args) {
        T* newnode = new T(args...);
        T* tail = _tail.load();
        T* front = _front.load();
        while(!_tail.compare_exchange_weak(tail, newnode)) {}
        if (tail == nullptr) {
            _front.store(newnode, std::memory_order_relaxed);
        } else {
            tail->next.store(newnode);
        }
        ++_len;
    }

    void pop() {
        T* front = _front.load();
        T* tail = _tail.load();
        if (_len <= 0) {
            if (front == nullptr) { 
                return void(_len = 0); 
            }
            while(1) {
                T* front = _front.load();
                if (front) {
                    if (_front.compare_exchange_weak(front, nullptr)) {
                        delete front;
                        front = nullptr;
                    }
                } else {
                    _front.compare_exchange_weak(front, nullptr);
                }
            }
            while(1) {
                T* tail = _tail.load();
                if (tail) {
                    if (_tail.compare_exchange_weak(tail, nullptr)) {
                        delete tail;
                        tail = nullptr;
                        return (void)(_len = 0);
                    }
                } else {
                    _tail.compare_exchange_weak(tail, nullptr);
                }
            }
        }
        while(1) {
            T* front = _front.load();
            T* tail = _tail.load();
            if (front) {
                T* tmp = front;
                if (_tail.compare_exchange_weak(tmp, nullptr)) {
                    if (_front.compare_exchange_weak(front, nullptr)) {
                        delete front;
                        front = nullptr;
                        return (void)(_len = 0);
                    }
                }
                if (_front.compare_exchange_weak(front, front->next)) {
                    delete front;
                    front = nullptr;
                    return (void)(--_len);
                }
            } else {
                return (void)(_len = 0);
            }
        }
        return (void)(_len = 0);
    }

    T* front() const {
        return _front.load();
    }

    bool is_empty() const {
        return _len.load() == 0;
    }
public:
    const int max_len = 200000;
private:
    std::atomic<int> _len;
    std::atomic<T*> _tail;
    std::atomic<T*> _front;
};