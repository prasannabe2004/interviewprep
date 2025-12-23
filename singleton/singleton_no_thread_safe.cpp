#include <iostream>
#include <thread>
#include <vector>

using namespace std;

class Singleton {
    public:
        static Singleton *CreateInstance()
        {
            if(ptr == nullptr)
            {
                ptr = new Singleton();
            }
            return ptr;
        }
        static void DestroyInstance()
        {
            if(ptr != nullptr)
                delete ptr;
            ptr = nullptr;
        }
    private:
        Singleton()
        {
            cout << "Singleton Created" << endl;
        }
        ~Singleton()
        {
            cout << "Singleton Destroyed" << endl;
        }
        static Singleton *ptr;
};

Singleton *Singleton::ptr = nullptr;

void work(int name)
{
    cout << "Thread " << std::this_thread::get_id() << endl; 
    Singleton::CreateInstance();
}

int main()
{
    std::vector<std::thread> threads;
    for(int i = 0; i < 10; i++)
    {
        //threads[i] = std::thread(work,i);
        threads.push_back(std::move(std::thread(work, i)));
        //threads.emplace_back(std::thread(work,i));
    }
    for(int i = 0; i < 10; i++)
    {
        if(threads[i].joinable())
            threads[i].join();
    }
    return 0;
}