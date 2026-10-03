#include "home_entry_settings.h"
#include "home_integration_package.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
namespace {
int regular(const std::string& path){int fd=open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_NONBLOCK);struct stat s{};if(fd>=0 && (fstat(fd,&s)!=0||!S_ISREG(s.st_mode))){close(fd);fd=-1;errno=EINVAL;}return fd;}
std::string body(bool on){return std::string("BetterFavoritesHome1\n")+(on?"1\n":"0\n");}
bool small(const std::string& path,std::string& data){int fd=regular(path);if(fd<0)return false;data.clear();char b[512];bool okay=true;for(;;){auto n=read(fd,b,sizeof(b));if(n<0&&errno==EINTR)continue;if(n<0){okay=false;break;}if(!n)break;data.append(b,n);if(data.size()>512){okay=false;break;}}if(close(fd)!=0)okay=false;return okay;}
uint32_t rotate(uint32_t v,unsigned n){return (v>>n)|(v<<(32-n));}
struct Sha {
 std::array<uint32_t,8> h{{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19}};
 uint64_t bytes=0;unsigned used=0;unsigned char block[64]{};
 void compress(){
  static const uint32_t k[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
  uint32_t w[64];for(unsigned i=0;i<16;++i)w[i]=uint32_t(block[i*4])<<24|uint32_t(block[i*4+1])<<16|uint32_t(block[i*4+2])<<8|block[i*4+3];
  for(unsigned i=16;i<64;++i){auto a=w[i-15],b=w[i-2];w[i]=(rotate(a,7)^rotate(a,18)^(a>>3))+w[i-16]+(rotate(b,17)^rotate(b,19)^(b>>10))+w[i-7];}
  auto v=h;for(unsigned i=0;i<64;++i){auto t=v[7]+(rotate(v[4],6)^rotate(v[4],11)^rotate(v[4],25))+((v[4]&v[5])^(~v[4]&v[6]))+k[i]+w[i];auto u=(rotate(v[0],2)^rotate(v[0],13)^rotate(v[0],22))+((v[0]&v[1])^(v[0]&v[2])^(v[1]&v[2]));for(unsigned j=7;j>0;--j)v[j]=v[j-1];v[4]+=t;v[0]=t+u;}for(unsigned i=0;i<8;++i)h[i]+=v[i];
 }
 void feed(const unsigned char* p,unsigned n){bytes+=n;while(n--){block[used++]=*p++;if(used==64){compress();used=0;}}}
 std::string finish(){const uint64_t bits=bytes*8;unsigned char pad=0x80;feed(&pad,1);pad=0;while(used!=56)feed(&pad,1);unsigned char tail[8];for(unsigned i=0;i<8;++i)tail[7-i]=bits>>(i*8);feed(tail,8);std::string result;const char* digits="0123456789abcdef";for(auto v:h)for(int i=7;i>=0;--i)result+=digits[(v>>(i*4))&15];return result;}
};
}
std::string homeFileSha256(const std::string& path){
 int fd=regular(path);if(fd<0)return {};struct stat before{},after{};bool okay=fstat(fd,&before)==0 && before.st_size<=4*1024*1024;Sha hash;unsigned char data[4096];
 while(okay){auto n=read(fd,data,sizeof(data));if(n<0&&errno==EINTR)continue;if(n<0){okay=false;break;}if(!n)break;hash.feed(data,n);if(hash.bytes>4*1024*1024)okay=false;}
 okay=okay&&fstat(fd,&after)==0&&before.st_size==after.st_size&&before.st_mtime==after.st_mtime;if(close(fd)!=0)okay=false;return okay?hash.finish():"";
}
bool loadHomeEntryPreference(const std::string& path,AppSettings& s,std::string& error){
 s.replaceStockFavorites=false;error.clear();std::string data;if(!small(path,data)){if(errno==ENOENT)return true;error="Cannot read Home preference; using OFF.";return false;}if(data==body(true)){s.replaceStockFavorites=true;return true;}if(data==body(false))return true;error="Malformed Home preference; using OFF.";return false;
}
bool setHomeEntryPreference(const std::string& path,bool enabled,AppSettings& saved,std::string& error){
 error.clear();struct stat old{};bool exists=lstat(path.c_str(),&old)==0;if((exists&&!S_ISREG(old.st_mode))||(!exists&&errno!=ENOENT)){error="Unsafe Home preference path. Previous setting kept.";return false;}
 std::string temp=path+".XXXXXX";int fd=mkstemp(&temp[0]);if(fd<0){error="Cannot stage Home preference. Previous setting kept.";return false;}
 const auto bytes=body(enabled);bool okay=fchmod(fd,0600)==0;size_t off=0;while(okay&&off<bytes.size()){auto n=write(fd,bytes.data()+off,bytes.size()-off);if(n<0&&errno==EINTR)continue;if(n<=0){okay=false;break;}off+=n;}if(okay)okay=fsync(fd)==0;if(close(fd)!=0)okay=false;
 std::string verified;okay=okay&&small(temp,verified)&&verified==bytes;
 struct stat current{};const bool now=lstat(path.c_str(),&current)==0;
 okay=okay&&(exists?now&&S_ISREG(current.st_mode)&&old.st_dev==current.st_dev&&old.st_ino==current.st_ino&&old.st_size==current.st_size&&old.st_mtime==current.st_mtime:!now&&errno==ENOENT);
 if(okay)okay=rename(temp.c_str(),path.c_str())==0;
 if(!okay){unlink(temp.c_str());error="Cannot save Home preference. Previous setting kept.";return false;}saved.replaceStockFavorites=enabled;return true;
}
HomeIntegrationStatus homeEntryStatus(const std::string& root,const std::string& app){
 std::string version;
 if(!small(root+"/.tmp_update/onionVersion/version.txt",version) ||
    (version!="v4.3.1-1\n" && version!="v4.3.1-1"))return HomeIntegrationStatus::Unavailable;
 const auto runtime=homeFileSha256(root+"/.tmp_update/runtime.sh");
 if(runtime!=homeOriginalRuntime&&runtime!=homeReturnRuntime)return HomeIntegrationStatus::Unavailable;
 std::string marker;std::string wanted="BetterFavoritesHomeInstalled1\nM6Home1\n";
 for(const auto& item:homePackageBinaries)wanted+=std::string(item.hash)+"\n";
 if(!small(app+"/home-integration.conf",marker)){
  if(errno!=ENOENT)return HomeIntegrationStatus::Unavailable;
  for(const auto& item:homePackageOriginals)if(homeFileSha256(root+"/.tmp_update/bin/"+item.name)!=item.hash)return HomeIntegrationStatus::Unavailable;
  return HomeIntegrationStatus::NotInstalled;
 }
 if(marker!=wanted)return HomeIntegrationStatus::Unavailable;
 for(const auto& item:homePackageBinaries)if(homeFileSha256(root+"/.tmp_update/bin/"+item.name)!=item.hash)return HomeIntegrationStatus::Unavailable;
 return HomeIntegrationStatus::Available;
}
bool homeEntryAvailable(const std::string& root,const std::string& app){
 return homeEntryStatus(root,app)==HomeIntegrationStatus::Available;
}
