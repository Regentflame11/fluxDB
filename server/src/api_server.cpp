#include "api_server.h"
#include <httplib.h>
#include <sstream>
#include <iostream>
static std::string js(const std::string& s){std::string o="\"";for(char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else o+=c;}return o+"\"";}
static std::string extract(const std::string& b,const std::string& k){std::string s="\""+k+"\"";size_t p=b.find(s);if(p==std::string::npos)return"";p=b.find(':',p);if(p==std::string::npos)return"";p=b.find('"',p);if(p==std::string::npos)return"";size_t e=b.find('"',p+1);if(e==std::string::npos)return"";return b.substr(p+1,e-p-1);}
struct APIServer::Impl{httplib::Server svr;};
APIServer::APIServer(KVStore& s,int p):store_(s),port_(p){impl_=new Impl();}
APIServer::~APIServer(){stop();delete impl_;}
void APIServer::start(){
    auto& svr=impl_->svr;
    svr.set_pre_routing_handler([](const httplib::Request& req,httplib::Response& res){res.set_header("Access-Control-Allow-Origin","*");res.set_header("Access-Control-Allow-Methods","GET,POST,OPTIONS");res.set_header("Access-Control-Allow-Headers","Content-Type");if(req.method=="OPTIONS"){res.status=204;return httplib::Server::HandlerResponse::Handled;}return httplib::Server::HandlerResponse::Unhandled;});
    svr.Get("/api/health",[](const httplib::Request&,httplib::Response& res){res.set_content("{\"status\":\"ok\"}","application/json");});
    svr.Get("/api/stats",[this](const httplib::Request&,httplib::Response& res){auto s=store_.stats();std::ostringstream o;o<<"{\"key_count\":"<<s.key_count<<",\"hits\":"<<s.hits<<",\"misses\":"<<s.misses<<",\"evictions\":"<<s.evictions<<",\"hit_rate\":"<<s.hit_rate<<",\"max_size\":"<<s.max_size<<"}";res.set_content(o.str(),"application/json");});
    svr.Get("/api/keys",[this](const httplib::Request&,httplib::Response& res){auto ks=store_.keys();std::string o="[";for(size_t i=0;i<ks.size();++i){if(i)o+=",";o+=js(ks[i]);}o+="]";res.set_content(o,"application/json");});
    running_=true;
    std::cout<<"[FluxDB] REST API on port "<<port_<<std::endl;
    svr.listen("0.0.0.0",port_);
}
void APIServer::stop(){if(running_){impl_->svr.stop();running_=false;}}
