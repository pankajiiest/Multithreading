
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <chrono>

template <typename T>

class BoundedBlockingQueue {
private:
    std::queue<T> q;
    std::mutex mtx;
    std::condition_variable cv_q_notEmpty;
    std::condition_variable cv_q_notFull;
    int cap = 0;
    
public:
    BoundedBlockingQueue(int capacity) : cap(capacity)
    {}
    
    void push(int num)
    {
        std::unique_lock<std::mutex> lock(mtx);
        
        cv_q_notFull.wait(lock, [this]() {
            return q.size() < cap;
        });
        
        std::cout<<"Pushing in a queue using pushThread "<<num<<std::endl;
        q.push(num);
        cv_q_notEmpty.notify_all();
    }
    
    void pop()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_q_notEmpty.wait(lock,[this](){
            return q.size() > 0;
        });
        
        std::cout<<"Popping from queue using popThread "<<q.front()<<std::endl;
        q.pop();
        cv_q_notFull.notify_all();
    }
    
};

void producer(BoundedBlockingQueue<int>& queue, int start, int end)
{
    for(int i = start; i <= end; i++)
    {
        queue.push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(BoundedBlockingQueue<int>& queue, int count)
{
    for(int i = 0; i < count; i++)
    {
        queue.pop();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


int main()
{
    BoundedBlockingQueue<int> queue(10);
    std::thread producerThread(producer,std::ref(queue),1,10);
    std::thread consumerThread1(consumer,std::ref(queue),5);
    std::thread consumerThread2(consumer,std::ref(queue),5);
    
    producerThread.join();
    consumerThread1.join();
    consumerThread2.join();
    return 0;
}

