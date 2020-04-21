#include "app-framework/Buffers/DequeBuffer.hh"

template <class T>
appframework::DequeBuffer<T>::DequeBuffer(){
  Configure();
}

template <class T>
void appframework::DequeBuffer<T>::Configure(){
  
  this->fAttributes.isBounded = true;
  this->fAttributes.isSearchable = false;
  
  fDeque = std::deque<T>();

  if(this->attributes().isBounded){
    fCapacity = 1000;
  }
  else{
    fCapacity = fDeque.max_size();
  }

  this->fPushTimeout = 1e6;
  this->fPopTimeout  = 1e7;
  fRetryTime   = 100;

  fPushRetries = this->fPushTimeout/fRetryTime;
  fPopRetries  = this->fPopTimout/fRetryTime;
}

template <class T>
int appframework::DequeBuffer<T>::push(const T& obj){

  int n_retries=0;
  while(full()){
    if(n_retries==fPushRetries){
      //could throw, but here choose to return -1
      //throw std::runtime_error("Reached Max Push Retries in DequeBuffer");
      return -1;
    }
    ++n_retries;
    usleep(fRetryTime);
  }
  
  std::lock_guard<std::mutex> lck(fMutex);
  if(full()) throw std::runtime_error("Full after check? Another thread accessing?");
  fDeque.push_back(obj);
  return size();
}

template <class T>
T appframework::DequeBuffer<T>::pop(){

  int n_retries=0;
  while(empty()){
    if(n_retries==fPopRetries){
      throw std::runtime_error("No Data: Reached Max Pop Retries in DequeBuffer");
      //return -1;
    }
    ++n_retries;
    usleep(fRetryTime);
  }

  std::lock_guard<std::mutex> lck(fMutex);
  if(empty()) throw std::runtime_error("Empty after check? Another thread accessing?");

  T obj(std::move(fDeque.front()));
  fDeque.pop_front();
  return T(obj);
}
