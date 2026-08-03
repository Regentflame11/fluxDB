#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
enum class CommandType { SET, GET, DEL, EXISTS, UNKNOWN };
struct Command { CommandType type=CommandType::UNKNOWN; std::vector<std::string> args; int ttl_seconds=-1; std::string error; };
class Protocol {
public:
    static Command parse(const std::string& line) {
        Command cmd; auto t=tokenize(line); if(t.empty()){cmd.error="empty";return cmd;}
        auto n=up(t[0]);
        if(n=="SET"){if(t.size()<3){cmd.error="SET needs key value";return cmd;}cmd.type=CommandType::SET;cmd.args={t[1],t[2]};if(t.size()>=5&&up(t[3])=="EX")try{cmd.ttl_seconds=std::stoi(t[4]);}catch(...){cmd.error="bad TTL";};}
        else if(n=="GET"){if(t.size()<2){cmd.error="GET needs key";return cmd;}cmd.type=CommandType::GET;cmd.args={t[1]};}
        else if(n=="DEL"){if(t.size()<2){cmd.error="DEL needs key";return cmd;}cmd.type=CommandType::DEL;cmd.args={t[1]};}
        else if(n=="EXISTS"){if(t.size()<2){cmd.error="EXISTS needs key";return cmd;}cmd.type=CommandType::EXISTS;cmd.args={t[1]};}
        else{cmd.error="Unknown: "+t[0];}
        return cmd;
    }
    static std::string ok()                        {return "+OK\r\n";}
    static std::string val(const std::string& v)   {return "$"+std::to_string(v.size())+"\r\n"+v+"\r\n";}
    static std::string null_bulk()                 {return "$-1\r\n";}
    static std::string integer(long long n)        {return ":"+std::to_string(n)+"\r\n";}
    static std::string error(const std::string& m) {return "-ERR "+m+"\r\n";}
    static std::string simple(const std::string& s){return "+"+s+"\r\n";}
private:
    static std::vector<std::string> tokenize(const std::string& l){std::vector<std::string>t;std::istringstream i(l);std::string s;while(i>>s)t.push_back(s);return t;}
    static std::string up(std::string s){std::transform(s.begin(),s.end(),s.begin(),::toupper);return s;}
};
