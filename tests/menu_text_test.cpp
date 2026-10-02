#include "menu_text.h"
#include <cassert>
#include <iostream>
int main(){
    auto measure=[](const std::string& s){int n=0;for(unsigned char c:s)if((c&0xc0)!=0x80)++n;return n*10;};
    auto lines=wrapMenuText("Keep the game file.",90,measure);assert((lines==std::vector<std::string>{"Keep the","game","file."}));
    lines=wrapMenuText("Pokémon超長名稱abcdefghijk",50,measure);
    std::string joined;for(const auto& line:lines){assert(measure(line)<=50);joined+=line;}
    assert(joined=="Pokémon超長名稱abcdefghijk");
    lines=wrapMenuText("First\n\nSecond",100,measure);assert(lines.size()==3&&lines[1].empty());
    assert(conciseMenuError("Favorites changed externally; removal cancelled.")=="Favorites changed. Nothing was removed. Reopen and try again.");
    assert(conciseMenuError("Cannot publish removal; source unchanged.").find("Nothing was removed")!=std::string::npos);
    assert(conciseMenuError("Removed, but directory sync failed; verified backup retained.")=="Favorite removed. Storage sync failed. Backup kept.");
    assert(conciseMenuError("Could not create and verify favorites backup; source unchanged.").find("Backup failed")!=std::string::npos);
    std::cout<<"Measured word wrapping, long words, UTF-8 boundaries and paragraph spacing: PASS\n";
}
