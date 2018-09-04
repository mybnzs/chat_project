#pragma once 
#include<vector>
#include<semaphore.h>

namespace server{

//类模板
//约定[head，tail）
template <typename T>
class BlockQueue{
public:
  BlockQueue(size_t s = 1024):data_(s),head_(0),tail_(0),size_(0){
    sem_init(&sem_data_,0,0);
    sem_init(&sem_blank_,0,s);
  }

  ~BlockQueue(){
    sem_destroy(&sem_data_);
    sem_destroy(&sem_blank_);
  }

  void PushBack(const T& value){
    sem_wait(&sem_blank_);
    data_[tail_++] = value;
    if(tail_ >= data_.size())
    {
      tail_ = 0;
    }
    ++size_;
    sem_post(&sem_data_);
  }

  void PopFront(T* value){
    sem_wait(&sem_data_);
    *value = data_[head_++];
    if(head_ >= data_.size())
    {
         head_ = 0;
    }
    --size_;
    sem_post(&sem_blank_);
  }
private:
  std::vector<T> data_;
  sem_t sem_data_;
  sem_t sem_blank_;
  size_t head_;
  size_t tail_;
  size_t size_;
  //由于我们此处只实现一个单生产者单消费者的BlockQueue
  //可以不加互斥锁
  //sem_t sem_lock_;
};

}//end server 
