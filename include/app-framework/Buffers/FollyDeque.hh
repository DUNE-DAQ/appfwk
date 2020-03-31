#ifndef app_framework_Buffers_FollyDeque_hh
#define app_framework_Buffers_FollyDeque_hh

#include "app-framework-base/Buffers/TemplatedBuffer.hh"

#include <deque>
#include <functional>

namespace appframework{

  template <class T>
  class FollyDeque : public TemplatedBuffer<T> {
    
  public:

    FollyDeque();

    void Configure();
    
    size_t size() { return fDeque.size(); }
    size_t capacity() { return fCapacity; }

    int push(const T&); ///push one on, return new size if successful, -1 if not
    int pop(T&);        ///pop one off, return new size if successful, -1 if not

    bool spy(size_t, T&); ///read value at position i into given T object. Return false if cannot
    bool read(T&);        ///read value at read position into given T object. Return false if cannot
    
    size_t getReadPos() { return fReadPos; }
    void resetReadPos() { fReadPos = 0; }

    bool isEmpty() { return (this->size()==0); }
    bool isFull() { return (this->size()==this->capacity()); }

  private:
    
    std::deque<T> fDeque;
    size_t        fCapacity;
    size_t        fReadPos;

  };

}


#endif //app_framework_Buffers_Buffer_hh
