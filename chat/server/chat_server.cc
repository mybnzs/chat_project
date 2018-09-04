#include"chat_server.h"
#include"../common/util.hpp"
#include"api.hpp"
#include<sstream>

namespace server{
 
int ChatServer::Start(const std::string& ip,short port)
{
  //启动服务器，并且进入事件循环
  //使用UDP作为我们的通信协议
  sock_ = socket(AF_INET,SOCK_DGRAM,0);
  if(sock_<0){
    perror("socket");
    return -1;
  }
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  addr.sin_port = htons(port);
  int ret = bind(sock_,(sockaddr*)&addr,sizeof(addr));
  if(ret<0){
    perror("bind");
    return -1;
  }
  LOG(INFO) << "Server start OK!\n";
  pthread_t productor,consumer;
  pthread_create(&productor,NULL,Product,this);
  pthread_create(&consumer,NULL,Consume,this);
  pthread_join(productor,NULL);
  pthread_join(consumer,NULL);
  return 0;
}

void* ChatServer::Product(void* arg){
  //生产者线程，主要做的事情读取socket中的数据，并且写入到BlockQueue
 ChatServer* server = reinterpret_cast<ChatServer*>(arg);
 while(true)
 {
  //咱们的recvMsg读取一次数据，并且写回到BlockQueue
  server->RecvMsg();
 }
 return NULL;

}

void* ChatServer::Consume(void* arg){
  ChatServer* server = reinterpret_cast<ChatServer*>(arg);
  while(true)
  {
    server->BroadCast();
  }
  return NULL;
}

int ChatServer::RecvMsg()
{
  //1.从socket中读取数据
  char buf[1024*5]={0};
  sockaddr_in peer;
  socklen_t len=sizeof(peer);
  ssize_t read_size=recvfrom(sock_,buf,sizeof(buf)-1,0,(sockaddr*)&peer,&len);
  if(read_size < 0)
  {
    perror("recvfrom");
    return -1;
  }
  buf[read_size] = '\0';
  LOG(INFO) <<"[Request]"<<buf<<"\n";
  //2.把数据插入到BlockQueue中
  Context context;
  context.str = buf;
  context.addr = peer;
  queue_.PushBack(context);
  return 0;
}

int ChatServer::BroadCast()
{
 //1.从队列中读取一个元素
  Context context;
  queue_.PopFront(&context);
 //2.对取出的字符串数据进行反序列化
  Data data;
  data.UnSerialize(context.str);
 //3.根据这个消息更新在线好友列表
  if(data.cmd == "quit"){
   //b）如果这个成员当前是一个下线的消息（command是一个特殊的值),把这样的一个成员从好友列表中删除 
    DelUser(context.addr,data.name);
  }else{
  //  a）如果这个成员当前不在好友列表中，加入进来
  //准备使用[]方法来操作 在线好友列表
    AddUser(context.addr,data.name);
  }

 //4.遍历在线列表，把这个数据依次发送给每一个客户端。
 //  （由于发送消息的用户也存在于好友列表中，因此在遍历列表时，也会给自己发送消息，从而达到发送者
 //    能够看到自己发送的消息的效果，但是这种实现方式不太好，完全可以控制客户端，不经过网络传输就实现这个功能
 //    此处还是用这个不好的方法来写，更优越的方法以后再实现）
 for(auto item:online_friend_list_)
 {
   LOG(INFO)<<item.first<<" "<<inet_ntoa(item.second.sin_addr)<<":"<<ntohs(item.second.sin_port);
   SendMsg(context.str,item.second);
 }
  LOG(INFO)<<"===========================================================\n";
  return 0;
}
void ChatServer::AddUser(sockaddr_in addr,const std::string& name){
  //1.这里的key相当于对ip地址和用户名连接
  //只所以使用name和ip地址进行拼接，本质上是因为仅仅使用ip可能会出现重复ip的情况
  std::stringstream ss;
  //tangzhong:1232234143
  ss<<name<<":"<<addr.sin_addr.s_addr;
  //[]map unordered_map 
  //1.如果不存在，就插入
  //2.如果存在，就修改
  //ValueType& operator[](const KeyType& key)
  //const ValueType& operator[](const KeyType& key) const 
  //
  //insert 返回值类型:迭代器，插入成功后的位置；bool 成功与失败
  online_friend_list_[ss.str()] = addr;
}

void ChatServer::DelUser(sockaddr_in addr,const std::string& name){

 std::stringstream ss;
 ss << name<<":"<<addr.sin_addr.s_addr;
 online_friend_list_.erase(ss.str());
 return;
}

void ChatServer::SendMsg(const std::string& data,sockaddr_in addr){
  //把这个数据通过sendto发送给客户端
  sendto(sock_,data.data(),data.size(),0,(sockaddr*)&addr,sizeof(addr));
  LOG(INFO)<<"[Response]"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<" "<<data<<"\n";
}

}// end server 
