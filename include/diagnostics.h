#ifndef BETTER_FAVORITES_DIAGNOSTICS_H
#define BETTER_FAVORITES_DIAGNOSTICS_H
#include <streambuf>
#include <string>
#ifndef BETTER_FAVORITES_VERSION
#define BETTER_FAVORITES_VERSION "development"
#endif
#ifndef BETTER_FAVORITES_COMMIT
#define BETTER_FAVORITES_COMMIT "unknown"
#endif
namespace diagnostics {
constexpr unsigned kSessionLimit=65536;
// Best-effort, bounded app-owned logging; never changes a handoff result.
void event(const std::string& message);
void rotate();
class Streams : public std::streambuf {
public:
    Streams(); ~Streams();
protected:
    int_type overflow(int_type c) override;
    int sync() override;
private:
    char line_[1024]; unsigned used_=0;
    std::streambuf *out_,*err_;
};
}
#endif
