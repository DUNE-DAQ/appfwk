#include "app-framework/Buffers/FollyDeque.hh"

template <class T>
appframework::FollyDeque<T>::FollyDeque(){}

template <class T>
void appframework::FollyDeque<T>::Configure(){
  
  this->fBounded = true;

  fDeque = std::deque<T>();

  if(this->isBounded()){
    fCapacity = 1000;
  }
  else{
    fCapacity = fDeque.max_size();
  }

  fReadPos=0;

}

template <class T>
int appframework::FollyDeque<T>::push(const T& obj){

  if(isFull()) return -1;

  fDeque.push_back(obj);
  return size();

}

template <class T>
int appframework::FollyDeque<T>::pop(T& obj){

  if(isEmpty()) return -1;

  obj = fDeque.front();
  fDeque.pop_front();
  if(fReadPos>0) --fReadPos;
  return size();

}

template <class T>
bool appframework::FollyDeque<T>::spy(size_t i, T& obj){

  if(size()<=i) return false;

  obj = fDeque[i];
  return true;

}

template <class T>
bool appframework::FollyDeque<T>::read(T& obj){

  if( fReadPos>=size() ) return false;

  obj = fDeque[fReadPos++];  
  return true;

}
