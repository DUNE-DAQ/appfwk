#include "app-framework/DequeBuffer.hh"

template <class T>
dunedaq::DequeBuffer<T>::DequeBuffer(){}

template <class T>
void dunedaq::DequeBuffer<T>::Configure(){
  
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
int dunedaq::DequeBuffer<T>::push(const T& obj){

  if(size()+1<capacity()){
    fDeque.push_back(obj);
    return size();
  }

  return -1;

}

template <class T>
int dunedaq::DequeBuffer<T>::pop(T& obj){

  if(size()>0){
    obj = fDeque.front();
    fDeque.pop_front(obj);
    return size();
  }

  return -1;

}
