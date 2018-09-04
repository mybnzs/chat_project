#pragma  once 
#include<ncurses.h>
#include<string>
#include<unordered_set>
#include<deque>

namespace client{

class Window{
public:
  Window();
  ~Window();
  //绘制标题窗口
  void DrawHeader(); 
  //绘制输入框
  void DrawInput();
  //绘制输出框
  void DrawOutput();
  //绘制好友列表
  void DrawFriendList();
  //往窗口中写字符串
  void PutStrToWin(WINDOW* win,int y,int x,const std::string& str);
  //从窗口中读字符串
  void GetStrFromWin(WINDOW* win,std::string* str);

  void AddMsg(const std::string& msg);
  void AddFriend(const std::string& friend_info);
  void DelFriend(const std::string& friend_info);

  WINDOW* header_win_;
  WINDOW* input_win_;
  WINDOW* output_win_;
  WINDOW* friend_list_win_;
  std::deque<std::string> msgs_;//适配器  queue是基于deque实现的
  //当前的在线好友列表
  std::unordered_set<std::string> friend_list_;
};

}//end client 
