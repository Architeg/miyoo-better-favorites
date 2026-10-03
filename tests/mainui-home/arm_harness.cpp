// QEMU-only: map exact patched MainUI bytes, substitute unavailable imported
// device APIs and App recent service. No device UI or mounted card is executed.
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <asm/unistd.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <new>
#include <fstream>
#include <glob.h>
#include <vector>
#include <stdexcept>

static const char* pending="/tmp/cmd_to_run.sh";
static const char* pref="/mnt/SDCARD/App/BetterFavoritesTest/home-entry.conf";
static const char* config="/mnt/SDCARD/App/BetterFavoritesTest/config.json";
static int failure=0, recent_calls=0, audiofix=1, cwd_mode=0, allocations=0, frees=0;
static long stage_fd=-1;
static bool tracking=false, throw_recent=false;
static unsigned exidx_address, exidx_count, syscall_address;
static std::string output;
static uint32_t row[24], grid[24], parent[72], vector_words[3], stack_slot;
static void write_file(const char* path, const std::string& text) {
    FILE* f=fopen(path,"wb"); assert(f); assert(fwrite(text.data(),1,text.size(),f)==text.size()); assert(fclose(f)==0);
}
static std::string read_file(const char* path) {
    std::ifstream f(path); return std::string((std::istreambuf_iterator<char>(f)),{});
}
extern "C" long raw_syscall(long,long,long,long);
extern "C" int abi_activate(void*);
extern "C" int abi_startup();
extern "C" long intercepted_syscall(long nr,long a,long b,long c) {
    if (failure==11 && nr==__NR_write) { failure=0; ((void(*)(const char*))0x17fc0)("foreign-other-writer\n"); }
    if (failure==1 && nr==__NR_write) return -5;
    if (failure==2 && nr==__NR_fsync) return -5;
    if (failure==3 && nr==__NR_close && a==stage_fd) { raw_syscall(nr,a,b,c); return -5; }
    if (failure==4 && nr==__NR_link) return -5;
    if (failure==5 && nr==__NR_link) { write_file(pending,"foreign-race\n"); }
    if (failure==6 && nr==__NR_open && (b&0100)) return -28;
    if (failure==7 && nr==__NR_fchmod) return -5;
    if (failure==8 && nr==__NR_write) { failure=9; return -4; }
    if (failure==9 && nr==__NR_write && c>1) c=1;
    if (failure==10 && nr==__NR_link) { write_file("/tmp/.offOrder","1"); return -5; }
    long result=raw_syscall(nr,a,b,c);
    if (nr==__NR_open && (b&0100) && result>=0) stage_fd=result;
    return result;
}
extern "C" void* __gnu_Unwind_Find_exidx(void* pc,int* count) {
    uintptr_t p=(uintptr_t)pc;
    if (p>=0x10000 && p<0x200000) { *count=exidx_count; return (void*)exidx_address; }
    typedef void* (*Fn)(void*,int*);
    static Fn real=(Fn)dlsym(RTLD_NEXT,"__gnu_Unwind_Find_exidx");
    assert(real); return real(pc,count);
}
static int fail_allocation=-1;
void* operator new(size_t n) {
    if (tracking && fail_allocation==0) throw std::bad_alloc();
    if (tracking && fail_allocation>0) --fail_allocation;
    void* p=malloc(n); if (!p) throw std::bad_alloc();
    if (tracking) ++allocations;
    return p;
}
void operator delete(void* p) noexcept { if (tracking && p) ++frees; free(p); }
void operator delete(void* p,size_t) noexcept { if (tracking && p) ++frees; free(p); }
extern "C" char* fake_getcwd(char* out,size_t size) {
    if (cwd_mode==1) return nullptr;
    if (cwd_mode==2) { assert(size>16); strcpy(out,"/different/path"); return out; }
    return getcwd(out,size);
}
extern "C" int fake_shm(void*,int index) { assert(index==13); return audiofix; }
extern "C" void fake_recent(void*,const char* label,const char* launch,void*,int a,int type,int enabled) {
    assert(!strcmp(label,"Better Favorites Test"));
    assert(!strcmp(launch,"/mnt/SDCARD/App/BetterFavoritesTest/launch.sh"));
    assert(a==0 && type==3 && enabled==1);
    assert(access(pending,F_OK)==0); ++recent_calls;
    if (throw_recent) throw std::bad_alloc();
}
extern "C" void* get_row(void*,int) { return row; }
static void jump(unsigned address, void* target) {
    uint32_t* p=(uint32_t*)address; p[0]=0xe51ff004; p[1]=(uintptr_t)target;
    __builtin___clear_cache((char*)p,(char*)(p+2));
}
static void map_binary(const char* file) {
    std::string data=read_file(file);
    auto* eh=(Elf32_Ehdr*)data.data();
    auto* ph=(Elf32_Phdr*)(data.data()+eh->e_phoff);
    for (unsigned i=0;i<eh->e_phnum;++i) {
        if (ph[i].p_type==PT_ARM_EXIDX) { exidx_address=ph[i].p_vaddr; exidx_count=ph[i].p_filesz/8; }
        if (ph[i].p_type!=PT_LOAD) continue;
        unsigned low=ph[i].p_vaddr&~4095U, end=(ph[i].p_vaddr+ph[i].p_memsz+4095)&~4095U;
        assert(mmap((void*)low,end-low,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=(void*)-1);
        memcpy((void*)ph[i].p_vaddr,data.data()+ph[i].p_offset,ph[i].p_filesz);
    }
    auto* sh=(Elf32_Shdr*)(data.data()+eh->e_shoff);
    const char* names=data.data()+sh[eh->e_shstrndx].sh_offset;
    Elf32_Shdr *plt=nullptr,*rel=nullptr,*symbols=nullptr;
    for (unsigned i=0;i<eh->e_shnum;++i) {
        const char* name=names+sh[i].sh_name;
        if (!strcmp(name,".plt")) plt=&sh[i];
        if (!strcmp(name,".rel.plt")) rel=&sh[i];
        if (!strcmp(name,".dynsym")) symbols=&sh[i];
    }
    assert(plt && rel && symbols);
    auto* syms=(Elf32_Sym*)(data.data()+symbols->sh_offset);
    const char* strings=data.data()+sh[symbols->sh_link].sh_offset;
    auto* reloc=(Elf32_Rel*)(data.data()+rel->sh_offset);
    for (unsigned i=0;i<rel->sh_size/8;++i) {
        const char* name=strings+syms[ELF32_R_SYM(reloc[i].r_info)].st_name;
        void* addr=!strcmp(name,"GetKeyShm") ? (void*)fake_shm : !strcmp(name,"getcwd") ? (void*)fake_getcwd : dlsym(RTLD_DEFAULT,name);
        if (addr) jump(plt->sh_addr+20+i*12,addr);
    }
    jump(0x1204c4,(void*)fake_recent);
    // Test substitute for stock Favorites construction; leave the real entry,
    // injected activation wrapper and real result epilogue in execution.
    // Assert replay left r3=0, return through stock epilogue.
    uint32_t fallback[]={0xe3530000,0x13a03063,0xea00003f}; // B 27970 from 2786c
    memcpy((void*)0x27864,fallback,sizeof(fallback));
    // Test injection lives only in memory, never in produced MainUI files.
    jump(syscall_address,(void*)intercepted_syscall);
    *(uint32_t*)0x15a34=0xe12fff1e; // startup-only test return, displaced mov fp remains real

    for (unsigned i=0;i<eh->e_phnum;++i) {
        if(ph[i].p_type!=PT_LOAD) continue;
        unsigned low=ph[i].p_vaddr&~4095U, end=(ph[i].p_vaddr+ph[i].p_memsz+4095)&~4095U;
        int prot=PROT_READ | (ph[i].p_flags&PF_W ? PROT_WRITE:0) | (ph[i].p_flags&PF_X ? PROT_EXEC:0);
        assert(mprotect((void*)low,end-low,prot)==0);
    }
    __builtin___clear_cache((char*)0x10000,(char*)0x200000);
}
static void reset() {
    unlink(pending); unlink("/tmp/.offOrder"); unlink(pref); unlink(config);
    write_file(pref,"BetterFavoritesHome1\n1\n");
    write_file(config,"{\"label\":\"Better Favorites Test\",\"icon\":\"icon.png\",\"launch\":\"launch.sh\",\"description\":\"wrapper\"}");
    stage_fd=-1; failure=0; audiofix=1; cwd_mode=0; fail_allocation=-1; recent_calls=0; throw_recent=false;
    memset(row,0,sizeof(row)); row[16]=1; row[19]=2;
    parent[1]=1; parent[2]=0; parent[3]=(uintptr_t)grid;
    vector_words[0]=(uintptr_t)&stack_slot; vector_words[1]=(uintptr_t)&stack_slot+4;
    vector_words[2]=vector_words[1]; stack_slot=(uintptr_t)parent;
    *(uint32_t**)0x17ff38=vector_words;
    grid[3]=1; grid[4]=3; grid[5]=4;
}
static int activate() {
    int a=allocations, f=frees;
    tracking=true;
    int result=abi_activate(grid);
    tracking=false;
    glob_t staging{};
    assert(glob("/tmp/.better-favorites-home.*",0,nullptr,&staging)==GLOB_NOMATCH);
    globfree(&staging);
    assert(allocations-a==frees-f); // adapter + native temporary strings freed
    return result;
}
static void clean_failure(const char* name) {
    assert(activate()==0); assert(access(pending,F_OK)!=0); assert(recent_calls==0);
    printf("PASS %s\n",name);
}
int main(int argc,char** argv) {
    assert(getenv("BF_MAINUI_HOME_TEST_ISOLATED") && !strcmp(getenv("BF_MAINUI_HOME_TEST_ISOLATED"),"1"));
    setbuf(stdout,nullptr);
    assert(argc==4); syscall_address=strtoul(argv[2],nullptr,0); output=argv[3];
    uint32_t vtable[8]{}; vtable[5]=(uintptr_t)get_row; grid[0]=(uintptr_t)vtable;
    uint32_t parent_vtable[8]{}; parent_vtable[6]=0x2edac; parent[0]=(uintptr_t)parent_vtable;
    static_assert(sizeof(std::string)==24,"native std::string ABI");
    new ((void*)&parent[45]) std::string();
    new ((void*)&grid[6]) std::string();
    map_binary(argv[1]);
    reset(); assert(activate()==3); assert(recent_calls==1);
    struct stat published{}; assert(stat(pending,&published)==0 && (published.st_mode&0777)==0700);
    write_file(output.c_str(),read_file(pending));
    assert(parent[2]==0 && parent[3]==(uintptr_t)grid && stack_slot==(uintptr_t)parent);
    // Execute original state serializer (cJSON implementation remains native).
    ((void(*)(void*,const char*))0x1c544)(vector_words,"/tmp/bf-native-state.json");
    auto saved=read_file("/tmp/bf-native-state.json");
    assert(saved.find("\"title\":\t1")!=std::string::npos && saved.find("\"type\":\t0")!=std::string::npos);
    assert(saved.find("\"currpos\":\t1")!=std::string::npos && saved.find("\"pagestart\":\t3")!=std::string::npos && saved.find("\"pageend\":\t4")!=std::string::npos);
    printf("PASS native dispatch, cleanup, result=3, Home stack and native state serialization\n");
    reset(); rename(pref,"/tmp/saved-pref"); symlink("/tmp/saved-pref",pref); clean_failure("symlink preference"); unlink(pref); rename("/tmp/saved-pref",pref);
    // No writer ever opens these FIFOs. A blocking open would hit alarm(2).
    for(const char* path : {pref,config,"/mnt/SDCARD/App/BetterFavoritesTest/better-favorites","/mnt/SDCARD/App/BetterFavoritesTest/launch.sh"}) {
        reset(); assert(rename(path,"/tmp/fifo-original")==0); assert(mkfifo(path,0600)==0);
        alarm(2);clean_failure("FIFO nonblocking regular-file refusal");alarm(0);
        assert(unlink(path)==0);assert(rename("/tmp/fifo-original",path)==0);
    }
    reset(); unlink(pref); clean_failure("default off");
    reset(); write_file(pref,"BetterFavoritesHome1\n0\n"); clean_failure("disabled");
    reset(); write_file(pref,"BetterFavoritesHome1\n1\nextra"); clean_failure("malformed preference");
    reset(); write_file(config,"{\"launch\":\"launch.sh\",}"); clean_failure("malformed app JSON");
    reset(); write_file(config,"{\"launch\":\"other.sh\"}"); clean_failure("unavailable app config");
    reset(); write_file(config,"{\"launch\":\"launch.sh\",\"launch\":\"launch.sh\"}"); clean_failure("duplicate config key");
    reset(); rename("/mnt/SDCARD/App/BetterFavoritesTest/launch.sh","/tmp/saved-launch.sh"); clean_failure("missing launcher"); rename("/tmp/saved-launch.sh","/mnt/SDCARD/App/BetterFavoritesTest/launch.sh");
    reset(); rename("/mnt/SDCARD/App/BetterFavoritesTest/better-favorites","/tmp/saved-binary"); clean_failure("missing binary"); rename("/tmp/saved-binary","/mnt/SDCARD/App/BetterFavoritesTest/better-favorites");
    reset(); { auto b=read_file("/mnt/SDCARD/App/BetterFavoritesTest/better-favorites"); write_file("/mnt/SDCARD/App/BetterFavoritesTest/better-favorites","corrupt ELF"); clean_failure("corrupt binary header"); write_file("/mnt/SDCARD/App/BetterFavoritesTest/better-favorites",b); }
    reset(); chmod("/mnt/SDCARD/App/BetterFavoritesTest/better-favorites",0600); clean_failure("non executable binary"); chmod("/mnt/SDCARD/App/BetterFavoritesTest/better-favorites",0700);
    reset(); write_file("/tmp/.offOrder","1"); clean_failure("shutdown precedence");
    reset(); parent[2]=-2; clean_failure("non Home");
    reset(); parent[3]=0; clean_failure("wrong Home grid");
    reset(); row[16]=99; clean_failure("different title");
    reset(); row[19]=3; clean_failure("different destination");
    reset(); cwd_mode=1; clean_failure("getcwd failure rejects alternate native command");
    reset(); cwd_mode=2; clean_failure("unexpected MainUI working directory");
    reset(); audiofix=0; clean_failure("incompatible native audio command falls back without changing audio");
    reset(); write_file(pending,"foreign\n"); assert(activate()==0); assert(read_file(pending)=="foreign\n"); assert(recent_calls==0); puts("PASS existing conflict preserved");
    reset(); failure=11; assert(activate()==0); assert(read_file(pending)=="foreign-other-writer\n"); assert(recent_calls==0); puts("PASS unrelated writer during transaction follows stock path and wins publication");
    reset(); failure=5; assert(activate()==0); assert(read_file(pending)=="foreign-race\n"); assert(recent_calls==0); puts("PASS atomic late conflict preserved");
    for (int fail : {1,2,3,4,6,7,10}) { reset(); failure=fail; clean_failure("publication IO failure"); }
    reset(); failure=8; assert(activate()==3); assert(recent_calls==1); puts("PASS EINTR and short writes");
    for(int n=0;n<3;++n) { reset(); fail_allocation=n; clean_failure("native allocation failure with EHABI cleanup"); }
    reset(); throw_recent=true; assert(activate()==3); assert(access(pending,F_OK)==0); puts("PASS post-publication exception retains committed handoff");
    // Ordinary command writer replay, with transaction inactive, still works.
    reset(); ((void(*)(const char*))0x17fc0)("ordinary Onion command\n"); assert(read_file(pending)=="ordinary Onion command\n"); puts("PASS stock writer isolation");
    reset(); {
        struct NativeConfig { std::string name,unused[2],launch; int type; uint32_t rest[7]{}; } cfg;
        static_assert(sizeof(NativeConfig)==128,"native config ABI");
        cfg.name="Better Favorites Test"; cfg.launch="/mnt/SDCARD/App/BetterFavoritesTest/launch.sh"; cfg.type=3;
        uint32_t action[4]{};
        ((void(*)(void*,void*,int,int))0x3984c)(action,&cfg,1,3);
        assert(((int(*)(void*,unsigned))0x18c54)(action,0)==3);
        ((void(*)(void*))0x398b4)(action);
        assert(recent_calls==1); puts("PASS ordinary native AppAction gate isolation");
    }
    const char* marker="/mnt/SDCARD/App/BetterFavoritesTest/home-diagnostics.conf";
    const char* log="/mnt/SDCARD/App/BetterFavoritesTest/home-diagnostics.log";
    write_file(marker,"BetterFavoritesHomeDiagnostics1\n1\n");unlink(log);
    reset();assert(abi_startup()==1);assert(read_file(log).find("event=mainui-startup")!=std::string::npos);
    assert(read_file(log).find("variant=MainUI-")!=std::string::npos);
    write_file(pref,"BetterFavoritesHome1\n0\n");clean_failure("diagnostics enabled with preference OFF");
    assert(read_file(log).find("event=stock-fallback reason=preference-off-or-malformed")!=std::string::npos);
    reset();assert(activate()==3);assert(read_file(log).find("event=publication reason=committed")!=std::string::npos);
    assert(read_file(log).find("event=native-dispatch reason=returned result=00000003")!=std::string::npos);
    reset();write_file(log,std::string(131072,'x'));assert(activate()==3);assert(read_file(log).size()==131072);
    unlink(log);assert(mkfifo(log,0600)==0);reset();alarm(2);assert(activate()==3);alarm(0);unlink(log);
    assert(mkdir(log,0700)==0);reset();assert(activate()==3);assert(rmdir(log)==0);
    puts("PASS optional startup/Home diagnostics, ABI replay, OFF fallback, bounded log and logging failure isolation");
    unlink(marker);unlink(log);
    write_file(marker,"malformed");reset();assert(activate()==3);assert(access(log,F_OK)!=0);
    unlink(marker);assert(mkfifo(marker,0600)==0);reset();alarm(2);assert(activate()==3);alarm(0);assert(access(log,F_OK)!=0);
    unlink(marker);puts("PASS malformed/FIFO diagnostics marker does not affect launch");
    reset(); unlink(pref); unlink(config);
    ((std::string*)&parent[45])->~basic_string();
    ((std::string*)&grid[6])->~basic_string();
    return 0;
}
