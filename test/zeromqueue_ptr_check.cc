#include "app-framework/Queues/ZeroMQueue.hh"

#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <memory>

std::mutex cout_mutex;

//======================================================================
struct Foo
{
    Foo(int x_) : x(x_) {}
    int x;
};

//======================================================================
size_t now_us()
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

//======================================================================
void print_receiver()
{
    appframework::ZeroMQueueSource<std::unique_ptr<Foo>> source("foo");
    for(int i=0; i<10; ++i){
        auto p=source.pop(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> l(cout_mutex);
        std::cout << "Received Foo(" << p->x << ")" << std::endl;
    }
}

//======================================================================
void receiver_speed_test(int N)
{
    appframework::ZeroMQueueSource<std::unique_ptr<Foo>> source("foo2");
    for(int i=0; i<N; ++i){
        auto p=source.pop(std::chrono::milliseconds(1000));
    }
}

//======================================================================
int main()
{
    {
        appframework::ZeroMQueueSink<std::unique_ptr<Foo>> sink("foo");
        
        std::thread t(print_receiver);
        
        for(int i=0; i<10; ++i){
            auto p=std::make_unique<Foo>(i);
            sink.push(std::move(p), std::chrono::milliseconds(1));
        }
        
        t.join();
    }

    {
        appframework::ZeroMQueueSink<std::unique_ptr<Foo>> sink("foo2");

        const size_t N=5000000;

        std::vector<std::unique_ptr<Foo>> v;
        for(int i=0; i<N; ++i){
            v.push_back(std::make_unique<Foo>(i));
        }

        std::thread t(receiver_speed_test, N);

        size_t start=now_us();

        for(int i=0; i<N; ++i){
            sink.push(std::move(v[i]), std::chrono::milliseconds(100));
            if(i%1000000==0){
                std::lock_guard<std::mutex> l(cout_mutex);
                std::cout << (i/1000000) << "m" << std::endl;
            }
        }

        t.join();

        size_t end=now_us();
        int64_t dur=end-start;
        double Mmsg_per_s=1e-6*N/(1e-6*dur);
        std::lock_guard<std::mutex> l(cout_mutex);
        std::cout << "Sent/received " << N << " messages in " << dur << "us. "
                  << Mmsg_per_s << "m msg/s" << std::endl;

    }

}
