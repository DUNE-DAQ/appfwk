#include "app-framework/Buffers/DequeBuffer.hh"

template <class T>
appframework::DequeBuffer<T>::DequeBuffer(){}

template <class T>
void appframework::DequeBuffer<T>::Configure(){
  
  this->fBounded = true;

  fDeque = std::deque<T>();

  if(this->isBounded()){
    fCapacity = 1000;
  }
  else{
    fCapacity = fDeque.max_size();
  }

}

template <class T>
int appframework::DequeBuffer<T>::push(const T& obj){

  if(size()+1<capacity()){
    fDeque.push_back(obj);
    return size();
  }

  return -1;

}

template <class T>
int appframework::DequeBuffer<T>::pop(T& obj){

  if(size()>0){
    obj = fDeque.front();
    fDeque.pop_front(obj);
    return size();
  }

  return -1;

}
