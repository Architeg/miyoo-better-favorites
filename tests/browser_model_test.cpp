#include "browser_model.h"
#include "ui_rows.h"
#include "navigation.h"
#include "favorite_removal.h"
#include <cassert>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <iostream>
std::string read(const std::string& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
 std::vector<Favorite> favorites;
 std::string original;
 for(const auto& label:{"01. Zebra","03. Alpha","02. Beta"}){Favorite f;f.label=label;f.launchPath="/mnt/SDCARD/Emu/GB/launch.sh";f.romPath="/mnt/SDCARD/Roms/GB/"+f.label+".zip";f.systemId=favorites.size()==1?"FC":"GB";f.systemLabel=f.systemId;f.sourceOffset=original.size();f.sourceRecord="{\"label\":\""+f.label+"\",\"extra\":7}\n";original+=f.sourceRecord;favorites.push_back(f);}
 AppSettings s;auto grouped=groupBrowserFavorites(favorites,s);auto rows=buildUiRows(grouped,s.groupByConsole);std::size_t selected=0;
 for(std::size_t i=0;i<rows.size();++i)if(rows[i].favorite&&rows[i].favorite->label=="01. Zebra")selected=i;
 const auto anchor=captureBrowserAnchor(rows,selected,0);
 s.groupByConsole=false;auto flat=groupBrowserFavorites(favorites,s);rows=buildUiRows(flat,s.groupByConsole);assert(rows.size()==3);
 assert(rows[0].favorite->label=="01. Zebra"&&rows[1].favorite->label=="02. Beta");
 for(const auto& row:rows)assert(row.type==UiRowType::Favorite);
 assert(previousSelectableRow(rows,0)==0&&nextSelectableRow(rows,2)==2);
 for(std::size_t i=0;i<rows.size();++i)assert(nextConsoleRow(rows,i)==i&&previousConsoleRow(rows,i)==i&&consoleHeaderRow(rows,i)==rows.size());
 assert(browserDisplayLabel(favorites[0],s)=="01. Zebra");
 Favorite plain;plain.label="1942";assert(browserDisplayLabel(plain,s)=="1942");
 assert(browserSortKey("Pokémon",s)=="Pokémon");
 assert(browserSortKey("03. Alpha",s)=="03. Alpha");
 s.showNumericPrefixes=false;assert(browserDisplayLabel(favorites[0],s)=="Zebra");
 auto sameOrder=groupBrowserFavorites(favorites,s);assert(sameOrder[0].favorites[0].label=="01. Zebra");
 s.sortMode=SortMode::AlphabeticalTitle;assert(browserSortKey("03. Alpha",s)=="alpha");assert(browserSortKey("Pokémon",s)=="pokémon");
 flat=groupBrowserFavorites(favorites,s);rows=buildUiRows(flat,false);assert(rows[0].favorite->label=="03. Alpha"&&rows[2].favorite->label=="01. Zebra");
 s.showNumericPrefixes=true;assert(browserDisplayLabel(favorites[0],s)=="01. Zebra");sameOrder=groupBrowserFavorites(favorites,s);assert(sameOrder[0].favorites[0].label=="03. Alpha");
 long first=0;restoreBrowserAnchor(anchor,rows,selected,first);assert(rows[selected].favorite->label=="01. Zebra");assert(first>=0&&first<=long(selected));
 // Same launch/ROM duplicates retain their exact selected source span.
 auto duplicate=favorites[0];duplicate.sourceOffset=original.size();duplicate.sourceRecord="{\"label\":\"01. Zebra\",\"duplicate\":true}\n";original+=duplicate.sourceRecord;favorites.push_back(duplicate);
 s.groupByConsole=true;grouped=groupBrowserFavorites(favorites,s);rows=buildUiRows(grouped,true);
 for(std::size_t i=0;i<rows.size();++i)if(rows[i].favorite&&rows[i].favorite->sourceOffset==duplicate.sourceOffset)selected=i;
 const auto duplicateAnchor=captureBrowserAnchor(rows,selected,0);
 s.groupByConsole=false;flat=groupBrowserFavorites(favorites,s);rows=buildUiRows(flat,false);restoreBrowserAnchor(duplicateAnchor,rows,selected,first);
 assert(rows[selected].favorite->sourceRecord==duplicate.sourceRecord);
 char dir[]="/tmp/better-favorites-model-test.XXXXXX";assert(mkdtemp(dir));const std::string path=std::string(dir)+"/favourite.json";
 {std::ofstream f(path,std::ios::binary);f<<original;}
 FavoritesSnapshot snapshot;std::string error,backup;assert(readFavoritesSnapshot(path,snapshot,error));assert(removeFavorite(path,snapshot,*rows[selected].favorite,backup,error));
 assert(read(path)==original.substr(0,duplicate.sourceOffset)&&read(backup)==original);
 unlink(path.c_str());unlink(backup.c_str());rmdir(dir);
 favorites.erase(std::remove_if(favorites.begin(),favorites.end(),[&](const Favorite& f){return f.launchPath==anchor.selected.launchPath&&f.romPath==anchor.selected.romPath;}),favorites.end());
 flat=groupBrowserFavorites(favorites,s);rows=buildUiRows(flat,false);restoreBrowserAnchor(anchor,rows,selected,first);assert(selected<rows.size()&&rows[selected].favorite->label=="02. Beta");
 assert(groupBrowserFavorites({},s).empty());rows.clear();restoreBrowserAnchor(anchor,rows,selected,first);assert(selected==0&&first==0);
 std::cout<<"Literal/title sort independence, flat/grouped navigation, identity preservation, exact duplicate removal after rebuild and missing/empty fallback: PASS\n";
}
