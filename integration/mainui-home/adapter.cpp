// Host-only prototype for the four audited Onion v4.3.1-1 MainUI hashes.
// No installation or activation is performed by this source.
#define _LARGEFILE64_SOURCE
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/unistd.h>

extern "C" long bf_syscall3(long, long, long, long);
extern "C" void* native_string_ctor(void*);
extern "C" void* native_string_assign(void*, const char*);
extern "C" void native_string_dtor(void*);
extern "C" void* native_app_ctor(void*, void*, int, int);
extern "C" int native_app_execute(void*, unsigned);
extern "C" void native_app_dtor(void*);

struct Transaction {
    int active, staged, committed;
    unsigned serial;
    long owner_tid;
    char temporary[80];
};
extern "C" { Transaction bf_transaction; }

extern "C" const char bf_variant[24] = "unassigned";
static const char diagnostic_flag[] = "/mnt/SDCARD/App/BetterFavoritesTest/home-diagnostics.conf";
static const char diagnostic_log[] = "/mnt/SDCARD/App/BetterFavoritesTest/home-diagnostics.log";
static char attempt[28];
static const char preference[] = "/mnt/SDCARD/App/BetterFavoritesTest/home-entry.conf";
static const char app_config[] = "/mnt/SDCARD/App/BetterFavoritesTest/config.json";
static const char binary[] = "/mnt/SDCARD/App/BetterFavoritesTest/better-favorites";
static const char launcher[] = "/mnt/SDCARD/App/BetterFavoritesTest/launch.sh";
static const char pending[] = "/tmp/cmd_to_run.sh";
static const char shutdown[] = "/tmp/.offOrder";
static const char enabled[] = "BetterFavoritesHome1\n1\n";
extern "C" const char bf_expected_command[] =
    "cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; "
    "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so   ./launch.sh";

static long call(long nr, long a=0, long b=0, long c=0) {
    long r;
    do { r=bf_syscall3(nr,a,b,c); } while (r==-4); // EINTR
    return r;
}
static unsigned length(const char* s, unsigned bound) {
    unsigned n=0; while (n<bound && s[n]) ++n; return n;
}
static bool equal(const char* a, const char* b, unsigned n) {
    for (unsigned i=0;i<n;++i) if (a[i]!=b[i]) return false;
    return true;
}
static int open_regular(const char* path) {
    // Never wait for a FIFO writer before descriptor-based regular-file validation.
    long fd=call(__NR_open,(long)path,O_RDONLY|O_NOFOLLOW|O_NONBLOCK,0);
    if (fd<0) return -1;
    struct stat64 st;
    if (call(__NR_fstat64,fd,(long)&st)<0 || !S_ISREG(st.st_mode)) {
        bf_syscall3(__NR_close,fd,0,0); return -1;
    }
    return (int)fd;
}
static bool read_small(const char* path, char* out, unsigned cap, unsigned& n) {
    int fd=open_regular(path); if (fd<0) return false;
    n=0; bool okay=true;
    while (n<cap) {
        long r=call(__NR_read,fd,(long)(out+n),cap-n);
        if (r<0) { okay=false; break; }
        if (!r) break;
        n+=(unsigned)r;
    }
    char extra;
    if (okay && n==cap && call(__NR_read,fd,(long)&extra,1)!=0) okay=false;
    // Do not retry close on EINTR: Linux may already have released the fd.
    if (bf_syscall3(__NR_close,fd,0,0)<0) okay=false;
    return okay;
}
static bool absent(const char* path) {
    struct stat64 st;
    return call(__NR_lstat64,(long)path,(long)&st)==-2; // ENOENT only
}

// Deliberately narrow prototype config schema: current App's four string keys.
// Unknown/duplicate keys or non-string values fall back; no general config engine.
struct Json {
    const char* p; const char* end;
    void space() { while (p<end && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n')) ++p; }
    bool take(char c) { space(); if (p==end || *p!=c) return false; ++p; return true; }
    bool string(char* out, unsigned cap) {
        if (!take('"')) return false;
        unsigned n=0;
        while (p<end) {
            unsigned ch=(unsigned char)*p++;
            if (ch=='"') { if (n>=cap) return false; out[n]=0; return true; }
            if (ch<32 || ch>=128) return false; // current audited schema is ASCII
            if (ch=='\\') {
                if (p==end) return false;
                ch=(unsigned char)*p++;
                if (ch=='u') {
                    unsigned code=0;
                    for (int i=0;i<4;++i) {
                        if (p==end) return false;
                        unsigned v=(unsigned char)*p++;
                        if (v>='0' && v<='9') v-='0';
                        else if (v>='a' && v<='f') v=v-'a'+10;
                        else if (v>='A' && v<='F') v=v-'A'+10;
                        else return false;
                        code=code*16+v;
                    }
                    // Non-ASCII strings are unused display metadata in this adapter.
                    if (code>127) return false; // fail closed on unsupported escaped metadata
                    ch=code;
                } else if (ch=='b') ch=8;
                else if (ch=='f') ch=12;
                else if (ch=='n') ch=10;
                else if (ch=='r') ch=13;
                else if (ch=='t') ch=9;
                else if (ch!='"' && ch!='\\' && ch!='/') return false;
            }
            if (!ch || n+1>=cap) return false;
            out[n++]=(char)ch;
        }
        return false;
    }
};
static bool config_valid() {
    char data[1024],key[32],value[1024]; unsigned n;
    if (!read_small(app_config,data,sizeof(data),n)) return false;
    Json j{data,data+n}; unsigned seen=0;
    if (!j.take('{')) return false;
    if (j.take('}')) return false;
    do {
        if (!j.string(key,sizeof(key)) || !j.take(':') || !j.string(value,sizeof(value))) return false;
        unsigned bit=0;
        if (length(key,32)==5 && equal(key,"label",6)) bit=1;
        else if (length(key,32)==4 && equal(key,"icon",5)) bit=2;
        else if (length(key,32)==6 && equal(key,"launch",7)) bit=4;
        else if (length(key,32)==11 && equal(key,"description",12)) bit=8;
        if (!bit || (seen&bit)) return false;
        if (bit==4 && (length(value,1024)!=9 || !equal(value,"launch.sh",10))) return false;
        seen|=bit;
        j.space();
        if (j.p<j.end && *j.p=='}') { ++j.p; break; }
        if (!j.take(',')) return false;
    } while (true);
    j.space(); return j.p==j.end && seen==15;
}
static bool available() {
    char b[24];
    if (!config_valid()) return false;
    int fd=open_regular(binary); if (fd<0) return false;
    long got=call(__NR_read,fd,(long)b,20);
    struct stat64 st;
    bool mode=call(__NR_fstat64,fd,(long)&st)==0 && (st.st_mode&0111);
    long closed=bf_syscall3(__NR_close,fd,0,0);
    if (got!=20 || !mode || closed<0 || !equal(b,"\177ELF\1\1\1",7) ||
        ((unsigned char)b[16]!=2 && (unsigned char)b[16]!=3) || b[17]!=0 || b[18]!=40 || b[19]!=0) return false;
    // Launcher is not run here. Stock AppAction will chmod it before execution.
    fd=open_regular(launcher); if (fd<0) return false;
    return bf_syscall3(__NR_close,fd,0,0)==0;
}

static void private_cleanup() {
    if (bf_transaction.temporary[0]) {
        // Only our exclusively-created staging path; NEVER unlink pending.
        if (call(__NR_unlink,(long)bf_transaction.temporary)==0)
            bf_transaction.temporary[0]=0;
    }
    bf_transaction.staged=0;
}
static void hex(char* dst, unsigned x) {
    static const char digits[]="0123456789abcdef";
    for (int i=7;i>=0;--i) { dst[i]=digits[x&15]; x>>=4; }
}
// Diagnostics are raw, bounded, best-effort syscalls, including before libc startup.
// Never fsync diagnostic writes or retry logging; they do not decide handoff.
static bool diagnostics_enabled() {
    char raw[40];
    const char wanted[]="BetterFavoritesHomeDiagnostics1\n1\n";
    long fd=bf_syscall3(__NR_open,(long)diagnostic_flag,O_RDONLY|O_NOFOLLOW|O_NONBLOCK,0);
    if(fd<0)return false;
    struct stat64 st;
    const bool regular=bf_syscall3(__NR_fstat64,fd,(long)&st,0)==0 && S_ISREG(st.st_mode) && st.st_size==sizeof(wanted)-1;
    long n=regular?bf_syscall3(__NR_read,fd,(long)raw,sizeof(raw)):-1;
    const long closed=bf_syscall3(__NR_close,fd,0,0);
    return closed==0 && n==sizeof(wanted)-1 && equal(raw,wanted,n);
}
static void append(char* out,unsigned& n,const volatile char* text) {
    for(unsigned i=0;text[i] && n<500;++i)out[n++]=text[i];
}
static void diagnostic(const char* event,const char* reason,int result=-99) {
    if(!diagnostics_enabled())return;
    char line[512];unsigned n=0;
    append(line,n,"M6Home1 variant=");append(line,n,bf_variant);
    append(line,n," pid=");char id[9]={};hex(id,(unsigned)call(__NR_getpid));append(line,n,id);
    append(line,n," attempt=");append(line,n,attempt[0]?attempt:"none");
    append(line,n," event=");append(line,n,event);append(line,n," reason=");append(line,n,reason);
    if(result!=-99){append(line,n," result=");hex(id,(unsigned)result);append(line,n,id);}
    line[n++]='\n';
    long fd=bf_syscall3(__NR_open,(long)diagnostic_log,O_WRONLY|O_APPEND|O_CREAT|O_NOFOLLOW|O_NONBLOCK,0600);
    if(fd<0)return;
    struct stat64 st;
    if(bf_syscall3(__NR_fstat64,fd,(long)&st,0)==0 && S_ISREG(st.st_mode) && st.st_size<=131072-512)
        (void)bf_syscall3(__NR_write,fd,(long)line,n);
    (void)bf_syscall3(__NR_close,fd,0,0);
}
extern "C" void bf_home_startup() { diagnostic("mainui-startup","entry"); }
static int stock(const char* reason) { diagnostic("stock-fallback",reason);return 0; }
extern "C" void bf_stage_native_command(const char* cmd) {
    const unsigned wanted=sizeof(bf_expected_command)-1;
    unsigned n=length(cmd,512);
    if (!bf_transaction.active || n>=512 || n<wanted || !equal(cmd,bf_expected_command,wanted)) { diagnostic("staging","command-format");return; }
    for (unsigned i=wanted;i<n;++i)
        if (cmd[i]!=' ' && cmd[i]!='\t' && cmd[i]!='\n') {diagnostic("staging","command-suffix");return;}
    if (bf_transaction.staged || bf_transaction.committed) return;
    char name[80]="/tmp/.better-favorites-home.00000000.00000000";
    hex(name+27,(unsigned)call(__NR_getpid));
    long fd=-1;
    for (int i=0;i<32 && fd<0;++i) {
        hex(name+36,++bf_transaction.serial);
        fd=call(__NR_open,(long)name,O_CREAT|O_EXCL|O_WRONLY|O_NOFOLLOW,0600); // CREAT|EXCL|WRONLY|NOFOLLOW
        if (fd<0 && fd!=-17) {diagnostic("staging","open-failed",fd);return;}
    }
    if (fd<0) {diagnostic("staging","name-exhausted");return;}
    unsigned name_n=length(name,sizeof(name));
    for (unsigned i=0;i<=name_n;++i) bf_transaction.temporary[i]=name[i];
    // Runtime executes the relocated command as a file, not via an explicit sh.
    bool okay=call(__NR_fchmod,fd,0700)==0;
    unsigned written=0;
    while (okay && written<n) {
        long r=call(__NR_write,fd,(long)(cmd+written),n-written);
        if (r<=0) { okay=false; break; }
        written+=(unsigned)r;
    }
    if (okay) okay=call(__NR_fsync,fd)==0;
    if (bf_syscall3(__NR_close,fd,0,0)<0) okay=false;
    if (okay) bf_transaction.staged=1;
    else private_cleanup();
    diagnostic("staging",okay?"ready":"write-failed");
}
extern "C" int bf_writer_is_owned() {
    return bf_transaction.active && bf_transaction.owner_tid==call(__NR_gettid);
}
extern "C" int bf_publish_gate() {
    if (!bf_writer_is_owned()) return 1; // ordinary stock AppAction unchanged
    if (!bf_transaction.staged || !absent(shutdown)) {diagnostic("publication","not-staged-or-shutdown");return 0;}
    // The absent check is NOT the publication guarantee. link() is the atomic
    // no-replace operation, including a foreign command created immediately here.
    long published=call(__NR_link,(long)bf_transaction.temporary,(long)pending);
    if (published!=0) {diagnostic("publication","no-replace-failed",published);return 0;}
    bf_transaction.committed=1;
    diagnostic("publication","committed");
    private_cleanup();
    return 1;
}

struct AppConfig {
    uint32_t name[6], unused[12], launch[6];
    int type; uint32_t rest[7];
};
struct AppAction { uint32_t words[4]; };
static_assert(sizeof(AppConfig)==128,"native config ABI");
static_assert(__builtin_offsetof(AppConfig,launch)==72,"native launch offset");
static_assert(sizeof(AppAction)==16,"native action ABI");

extern "C" int bf_home_activate(void* grid, void* row) {
    if (bf_transaction.active || bf_transaction.temporary[0] || !grid || !row) return 0;
    uint32_t* r=(uint32_t*)row;
    if (r[16]!=1 || r[19]!=2) return 0; // stock numeric Favorites title and target
    uint32_t* stack=*(uint32_t**)0x17ff38;
    if (!stack || !stack[0] || stack[1]<=stack[0] ||
        (stack[1]-stack[0])%4 || stack[1]-stack[0]>256) return 0;
    uint32_t* parent=*(uint32_t**)(stack[1]-4);
    if (!parent || parent[2]!=0 || parent[3]!=(uint32_t)grid) return 0;
    hex(attempt,(unsigned)call(__NR_getpid));attempt[8]='.';
    struct {long sec,usec;} now{};
    (void)call(__NR_gettimeofday,(long)&now,0);
    hex(attempt+9,(unsigned)now.sec);attempt[17]='.';hex(attempt+18,(unsigned)now.usec);attempt[26]=0;
    char raw[32]; unsigned n;
    const bool on=read_small(preference,raw,sizeof(raw),n) && n==sizeof(enabled)-1 && equal(raw,enabled,n);
    diagnostic("home-favorites",on?"preference-on":"preference-off-or-malformed");
    if(!on)return stock("preference-off-or-malformed");
    if(!available())return stock("app-unavailable-or-invalid");
    if(!absent(shutdown))return stock("shutdown");
    // Early check is not a concurrency guarantee; publication remains link/no-replace.
    if(!absent(pending))return stock("pending-command");
    bf_transaction.staged=bf_transaction.committed=0;
    bf_transaction.temporary[0]=0;
    bf_transaction.owner_tid=call(__NR_gettid);
    if (bf_transaction.owner_tid<=0) return stock("thread-identity");
    bf_transaction.active=1;
    AppConfig config{}; AppAction action{};
    bool name_ready=false, launch_ready=false, action_ready=false;
    try {
        native_string_ctor(config.name); name_ready=true;
        native_string_ctor(config.launch); launch_ready=true;
        native_string_assign(config.name,"Better Favorites Test");
        native_string_assign(config.launch,launcher);
        config.type=3;
        native_app_ctor(&action,&config,1,3); action_ready=true;
        const int native_result=native_app_execute(&action,0);
        diagnostic("native-dispatch","returned",native_result);
    } catch (...) {
        diagnostic("native-dispatch","exception");
        // Native exception paths destroy their local strings before unwinding.
        // Once publication commits, NEVER report fallback or unlink pending.
        // Publication is the success criterion, even if recent registration throws.
    }
    if (action_ready) native_app_dtor(&action); // regular dtor: stack object
    if (launch_ready) native_string_dtor(config.launch);
    if (name_ready) native_string_dtor(config.name);
    bool committed=bf_transaction.committed;
    private_cleanup();
    bf_transaction.active=0;
    diagnostic(committed?"redirect":"stock-fallback",committed?"published":"dispatch-not-committed",committed?3:0);
    return committed ? 3 : 0;
}
