#pragma once 
#include<string.h>
#include<unordered_map>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"block_queue.hpp"
#include"api.hpp"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in  sockaddr_in; 


namespace server{

struct Context{
    std::string str;
    sockaddr_in addr;
};

class ChatServer{
  public:
    int Start(const std::string& ip,short port);
    //接收消息
    int RecvMsg();
    //广播消息
    int BroadCast();

  private:
    static void* Consume(void*);
    static void* Product(void*);

    void AddUser(sockaddr_in addr,const std::string & name);
    void DelUser(sockaddr_in addr,const std::string& name);
    void SendMsg(const std::string& str,sockaddr_in addr);
    //key内容用户身份标识，ip+昵称
    //value ip+端口号（stuct sockaddr_in)
    std::unordered_map<std::string,sockaddr_in> online_friend_list_;
    //todo 此处需要使用一个堵塞队列来作为生产者消费者模型的交易场所
    //暂定队列中元素类型为std::string 
    BlockQueue<Context> queue_;
    int sock_;//服务器进行绑定的文件描述符

  };
} //end server 
