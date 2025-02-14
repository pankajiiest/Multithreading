
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <chrono>
#include <unordered_map>
#include <vector>

class LRUCache
{
    private:
        int cap;
        std::deque<std::pair<int,int>> keyValuePairList;
        std::unordered_map<int,std::deque<std::pair<int,int>>::iterator> cacheMap;
        std::mutex mtx;
        std::condition_variable cv;
        std::thread evictionThread;
        int shutDown = false;
        
        void evict()
        {
            std::unique_lock<std::mutex> lock(mtx);
            while(!shutDown)
            {
                cv.wait_for(lock,std::chrono::seconds(10));
                
                if(!keyValuePairList.empty())
                {
                    std::cout<<"Evicting least Used "<<keyValuePairList.back().first<<" "<<keyValuePairList.back().second<<std::endl;
                   cacheMap.erase(keyValuePairList.back().first);
                   keyValuePairList.pop_back();
                }
                  
            
                // cv.notify_all();
            }
        }
        
    public:
        LRUCache(int capacity) : cap(capacity), evictionThread(&LRUCache::evict,this)
        {}
        
        ~LRUCache()
        {
            std::cout<<"Calling Destructor"<<std::endl;
            {
                std::unique_lock<std::mutex> lock(mtx);
            
            
                shutDown = true;
            }
            
            cv.notify_one();
            evictionThread.join();
        }
        
        int get(int key)
        {
            std::unique_lock<std::mutex> lock;
            
            if(cacheMap.find(key) == cacheMap.end())
            {
                return -1;
            }
            
            auto it = cacheMap[key];
            int value = it->second;
            
            keyValuePairList.erase(it);
            keyValuePairList.push_front({key,value});
            cacheMap[key] = keyValuePairList.begin();
            
            return value;
        }
        
        void put(int key, int value)
        {
            std::unique_lock<std::mutex> lock;
            
            if(keyValuePairList.size() >= cap)
            {
                cacheMap.erase(keyValuePairList.back().first);
                keyValuePairList.pop_back();
            }
            else if(cacheMap.find(key) != cacheMap.end())
            {
                auto it = cacheMap[key];
                keyValuePairList.erase(it);
                cacheMap.erase(key);
            }
            
            keyValuePairList.push_front({key,value});
            cacheMap[key] = keyValuePairList.begin();
            if(keyValuePairList.size() >= cap)
                cv.notify_one();
        }
    
};

void ProcessLRU(LRUCache& cache, int threadID)
{
    for(int i = 0; i < 5; i++)
    {
        int key = threadID*10 + i;
        cache.put(key,2*key);
        
        
        
        std::cout<<"Putting value in LRUCache from threadID " << threadID << " "<<key<<std::endl;
        
        int val = cache.get(key);
        std::cout<<"Getting value from LRUCache from threadID "<< threadID << " "<<key<<" "<<val<<std::endl;
    }
    
}

int main()
{
    
    LRUCache cache(10);
    
    std::vector<std::thread> threadList;
    
    for(int i = 1; i <= 3; i++)
    {
        threadList.push_back(std::thread(ProcessLRU,std::ref(cache),i));
    }
    
    for(auto& th : threadList)
    {
        th.join();
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
