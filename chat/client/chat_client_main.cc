#include "chat_client.h"
#include"../server/api.hpp"
#include "../common/util.hpp"
#include "window.h"
#include<signal.h>

client::ChatClient* g_client = NULL;
client::Window* g_window = NULL;
pthread_mutex_t g_lock;

void* Send(void* arg)
{
  //发送线程：读取输入数据，发送给服务器
  (void *)arg;
  while(1)
  {
    std::string msg;
    pthread_mutex_lock(&g_lock);
    g_window->DrawInput();
    pthread_mutex_unlock(&g_lock);

    g_window->GetStrFromWin(g_window->input_win_,&msg);
    g_client->SendMsg(msg);
  }
  return NULL;
}
void* Recv(void* arg)
{
  //接收数据：从服务器中读取数据，显示到界面上
  (void*)arg;
  while(1)
  {
    //此处需要绘制两个窗口，因为收到一条数据后output窗口需要更新，好友列表窗口也可能需要更新
    pthread_mutex_lock(&g_lock);
    g_window->DrawOutput();
    g_window->DrawFriendList();
    pthread_mutex_unlock(&g_lock);
    server::Data data;
    g_client->RecvMsg(&data);
    if(data.cmd=="quit")
    {
      //删除的方式和插入的方式能够对应上
      g_window->DelFriend(data.name+"|"+data.school);
    }else{
    //相当于好友列表的显示格式是如下：
    //老马|陕科大
      g_window->AddFriend(data.name+"|"+data.school);
      g_window->AddMsg(data.name+": "+data.msg);
    }
  }
  return NULL;
}

void Quit(int sig)
{
  (void)sig;
  //此处delete最主要的目的是为了执行析构函数。尤其是g_window的析构函数，因为这个函数
  //对ncurses进行了销毁动作。如果不做这个动作，很可能会导致终端显示混乱
  delete  g_window;
  delete  g_client;
  exit(0);
}
//goooogle常见的c++开源库
//protobuf：用来序列化
//glog：打日志
//gflag：配置项管理
//grpc：RPC框架
//gtest:单元测试框架
void Run(const std::string& ip,short port)
{
  //0.扑捉SIGINT信号
  signal(SIGINT,Quit);
  pthread_mutex_init(&g_lock,NULL);
  //1.初始化客户端核心模块
  g_client = new client::ChatClient();
  int ret = g_client->Init(ip,port);
  if(ret<0)
  {
    //此处其实是模仿了glog
    LOG(ERROR)<<"client Init failed!\n";
    return;
  }
  //2.提示用户输入用户名和学校
  std::string name,school;
  std::cout<<"请输入用户名："<<std::endl;
  std::cin>>name;
  std::cout<<"请输入学校："<<std::endl;
  std::cin>>school;
  g_client->SetUserInfo(name,school);

  //3.初始化用户界面模块
  g_window = new client::Window();
  g_window->DrawHeader();
  //4.创建两个线程，让线程完成数据读取和发送
  pthread_t stid,rtid;
  // a）发送线程：读取输入数据，发送给服务器
  pthread_create(&stid,NULL,Send,NULL);
  // b）接收线程：从服务器中读取数据，显示到界面上
  pthread_create(&rtid,NULL,Recv,NULL);
  pthread_join(stid,NULL);
  pthread_join(rtid,NULL);
  //5.对进程退出时进行处理
  delete g_window;
  delete g_client;
  pthread_mutex_destroy(&g_lock);
}

int main(int argc,char* argv[])
{
  if(argc!=3)
  {
    LOG(ERROR)<<"Usage ./chat_client [ip][port]\n";
    return 1;
  }

  Run(argv[1],atoi(argv[2]));
  return 0;
}
