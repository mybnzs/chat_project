#include"chat_client.h"
#include"../server/api.hpp"

namespace client{
  //低版本，ip和port是通过命令行参数来传的
  //如果需要从配置文件中读取ip和port，如何修改？可以利用json来组织配置格式
  int ChatClient::Init(const std::string& server_ip,short server_port){
    sock_ = socket(AF_INET,SOCK_DGRAM,0);
    if(sock_<0)
    {
      perror("socket");
      return -1;
    }
    
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr_.sin_port = htons(server_port);

    return 0;
  }

  int ChatClient::SetUserInfo(const std::string& name,const std::string& school){
    name_ = name;
    school_ = school;

    return 0;

  }

  //用户输入了hehe ，msg="hehe"
  void ChatClient:: SendMsg(const std::string& msg)
  {
    //按照下面这种方式来发送时错误的
    //客户端和服务器交互的接口要严格按照服务器提供的api来操作
    //这样的约定就相当于自定制了一种应用层协议
    //isendto(sock_,msg.c_str(),msg.size(),0,(sockaddr*)&server_addr_,sizeof(server_addr_));
    //数据的准备工作
    server::Data data;
    data.name = name_;
    data.school = school_;
    data.msg = msg;
    //以下的逻辑，相当于约定了用户输入什么样的内容就表示要进行下线
    if(data.msg=="quit"||data.msg == "exit"||data.msg == "Q")
    {
      data.cmd = "quit";
    }
    std::string str;
    data.Serialize(&str);

    //真正的数据发送
    sendto(sock_,str.c_str(),str.size(),0,(sockaddr*)&server_addr_,sizeof(server_addr_));
    return;
  }

  void ChatClient::RecvMsg(server::Data* data) 
  {
    char buf[1024*10]={0};
    //由于对端的ip端口号就是服务器的ip端口号，所以此处没有必要再获取一遍对端的ip
    ssize_t read_size = recvfrom(sock_,buf,sizeof(buf)-1,0,NULL,NULL);
    if(read_size < 0)
    {
      perror("recvfrom");
      return;
    }
    buf[read_size]='\0';
    data->UnSerialize(buf);
    return;
  }
}//end client 


//////////////////////////////////////////////////////////////////////////////////////// 
///先实现一个简易版本的客户端，通过这个简易版本的客户端来测试客户端和服务器的代码///////
////////////////////////////////////////////////////////////////////////////////////////
//这个客户端不是我们最终的客户端，只是用来测试客户端和服务的代码
#ifdef TEST_CHAT_CLIENT

#include<iostream>

void* Send(void* arg)
{

  //循环发送数据
  client::ChatClient* client = reinterpret_cast<client::ChatClient*>(arg);
  while(true)
  {
    std::string str;
    std::cin>>str;
    client->SendMsg(str);
  }
  return NULL;
}

void* Recv(void* arg)
{
   client::ChatClient* client = reinterpret_cast<client::ChatClient*>(arg);
  //循环接收数据
  while(true)
  {
    server::Data data;
    client->RecvMsg(&data);
    //[tz|xigongda] hehe
    std::cout<<"["<<data.name<<"|"<<data.school<<"]"<<data.msg<<"\n";
  }
  return NULL;
}

int main(int argc,char* argv[])
{
  if(argc!=3)
  {
    printf("Usage ./client [ip][port]\n");
    return 1;
  }
  client::ChatClient client;
  client.Init(argv[1],atoi(argv[2]));
  std::string name,school;
  std::cout<<"请输入用户名：";
  std::cin >> name;
  std::cout<<"输入学校：";
  std::cin >> school;
  client.SetUserInfo(name,school);

  //创建两个线程，分别用于发送数据和接收数据
  pthread_t stid,rtid;
  pthread_create(&stid,NULL,Send,&client);
  pthread_create(&rtid,NULL,Recv,&client);
  pthread_join(stid,NULL);
  pthread_join(rtid,NULL);
  return 0;
}

#endif
