#pragma once 
#include<string>
#include<jsoncpp/json/json.h>

namespace server{

//此处的Data相当于服务器给客户端提供的API
struct Data{
  std::string name;
  std::string school;
  std::string msg;
  std::string cmd;

  void Serialize(std::string* output) 
  {
    //char buf[1024]={0};
    //sprintf(buf,"{name:%s,school:%s,msg:%s,cmd:&s}",name.c_str(),school.c_str(),msg.c_str(),cmd.c_str());
    //srv *output = buf;
    //可以吧Json::Value 近似理解成一个unordered_map
    Json::Value value;
    value["name"] = name;
    value["school"] = school;
    value["msg"] = msg;
    value["cmd"] = cmd;
    Json::FastWriter writer;
    *output = writer.write(value);
    return;
  }

  void UnSerialize(const std::string& input)
  {
    Json::Value value;
    Json::Reader reader;
    reader.parse(input,value);
    //为了让代码更严谨，可以使用isString/isinit来判断
    name = value["name"].asString();
    school = value["school"].asString();
    msg = value["msg"].asString();
    cmd = value["cmd"].asString();

    return;
  }

};

} //end server 
