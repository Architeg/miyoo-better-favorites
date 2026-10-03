#include "diagnostics.h"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
namespace diagnostics {
namespace {
std::string path(){const char* p=std::getenv("BETTER_FAVORITES_LOG");return p&&*p?p:"/mnt/SDCARD/App/BetterFavoritesTest/better-favorites.log";}
bool regular(int fd,struct stat& s){return fd>=0&&fstat(fd,&s)==0&&S_ISREG(s.st_mode);}
}
void event(const std::string& message){
 const std::string p=path(); int fd=open(p.c_str(),O_WRONLY|O_CREAT|O_APPEND|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC,0600);struct stat s;
 if(!regular(fd,s)){if(fd>=0)close(fd);return;}
 // Single serial launcher/app ownership is required. Publisher invocations run
 // only after the browser has exited; no logging helper stays during gameplay.
 std::string line=message.substr(0,1024);for(char& c:line)if(c=='\r'||c=='\n')c=' ';line+='\n';
 if(s.st_size>=0&&static_cast<unsigned long long>(s.st_size)+line.size()<=kSessionLimit){
  const char* b=line.data();std::size_t left=line.size();while(left){ssize_t n=write(fd,b,left);if(n<=0)break;b+=n;left-=static_cast<std::size_t>(n);}
 }
 close(fd);
}
void rotate(){
 const std::string p=path(),previous=p.substr(0,p.find_last_of('/'))+"/better-favorites.previous.log";
 struct stat s;int fd=open(p.c_str(),O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
 if(fd>=0){if(!regular(fd,s)||s.st_size<0){close(fd);return;}char data[kSessionLimit];ssize_t n=read(fd,data,sizeof(data));close(fd);if(n<0)return;
  const std::string tmp=previous+".stage.XXXXXX";std::string name=tmp;int out=mkstemp(&name[0]);if(out<0)return;
  bool ok=fchmod(out,0600)==0;ssize_t written=0;while(ok&&written<n){ssize_t w=write(out,data+written,n-written);if(w<=0)ok=false;else written+=w;}
  if(close(out)!=0)ok=false;
  struct stat old;if(lstat(previous.c_str(),&old)==0&&!S_ISREG(old.st_mode))ok=false;
  if(ok&&rename(name.c_str(),previous.c_str())!=0)ok=false;
  unlink(name.c_str());
  if(!ok)return;
 }
 // Never follow a symlink/nonregular file, including an existing FIFO.
 fd=open(p.c_str(),O_WRONLY|O_CREAT|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC,0600);
 if(regular(fd,s))ftruncate(fd,0);
 if(fd>=0)close(fd);
}
Streams::Streams():out_(std::cout.rdbuf(this)),err_(std::cerr.rdbuf(this)){}
Streams::~Streams(){sync();std::cout.rdbuf(out_);std::cerr.rdbuf(err_);}
std::streambuf::int_type Streams::overflow(int_type c){if(traits_type::eq_int_type(c,traits_type::eof()))return traits_type::not_eof(c);char ch=traits_type::to_char_type(c);if(ch=='\n'){if(used_){event(std::string(line_,used_));used_=0;}}else if(used_<sizeof(line_)){line_[used_++]=ch;}return c;}
int Streams::sync(){return 0;} // std::cerr's unitbuf must not fragment each line.
}
