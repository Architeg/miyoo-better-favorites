#include "navigation.h"
#include "ui_rows.h"
#include <cassert>
#include <iostream>
int main(){
    auto empty=buildUiRows({});assert(firstSelectableRow(empty)==0);
    assert(nextSelectableRow(empty,0)==0 && previousSelectableRow(empty,0)==0);
    assert(nextConsoleRow(empty,0)==0 && previousConsoleRow(empty,0)==0);
    SystemGroup a,b,c;a.label="GB";b.label="Empty";c.label="FC";
    a.favorites.resize(2);c.favorites.resize(1);
    auto rows=buildUiRows({a,b,c});
    assert(firstSelectableRow(rows)==1);
    assert(previousSelectableRow(rows,1)==1 && nextSelectableRow(rows,5)==5);
    assert(nextSelectableRow(rows,2)==5 && previousSelectableRow(rows,5)==2);
    assert(nextConsoleRow(rows,1)==5 && nextConsoleRow(rows,5)==5);
    assert(previousConsoleRow(rows,5)==1 && previousConsoleRow(rows,1)==1);
    assert(consoleHeaderRow(rows,1)==0 && consoleHeaderRow(rows,5)==4);
    auto flat=buildUiRows({a,c},false);assert(flat.size()==3);
    assert(nextSelectableRow(flat,0)==1 && previousSelectableRow(flat,2)==1);
    assert(nextSelectableRow(flat,2)==2 && previousSelectableRow(flat,0)==0);
    for(std::size_t i=0;i<flat.size();++i)assert(nextConsoleRow(flat,i)==i && previousConsoleRow(flat,i)==i && consoleHeaderRow(flat,i)==flat.size());
    auto one=buildUiRows({c});assert(firstSelectableRow(one)==1);
    assert(nextSelectableRow(one,1)==1 && previousSelectableRow(one,1)==1);
    assert(nextConsoleRow(one,1)==1 && previousConsoleRow(one,1)==1);

    // Flat pages: six 60px rows; partial pages clamp and never wrap.
    SystemGroup many;many.favorites.resize(20);
    auto flatPages=buildUiRows({many},false);
    assert(pageSelectableRow(flatPages,0,0,1,360,50)==6);
    assert(pageSelectableRow(flatPages,6,1,-1,360,50)==0);
    assert(pageSelectableRow(flatPages,18,14,1,360,50)==19);
    assert(pageSelectableRow(flatPages,19,14,1,360,50)==19);
    assert(pageSelectableRow(flatPages,0,0,-1,360,50)==0);
    assert(pageSelectableRow(flatPages,7,2,1,360,50,true)==7);
    assert(pageSelectableRow(flatPages,7,2,-1,360,50,true)==7);
    assert(pageSelectableRow(empty,0,0,1,360,50)==0);
    assert(pageSelectableRow(one,1,0,1,360,50)==1);
    assert(pageSelectableRow(one,1,0,-1,360,50)==1);
    SystemGroup first,second;first.favorites.resize(8);second.favorites.resize(4);
    auto groupedPages=buildUiRows({first,second});
    assert(pageSelectableRow(groupedPages,1,0,1,360,50)==7);
    // Sticky heading subtracts 50px; crossing a heading counts its height.
    assert(pageSelectableRow(groupedPages,7,2,1,360,50)==12);
    assert(pageSelectableRow(groupedPages,12,7,-1,360,50)==7);
    assert(pageSelectableRow(groupedPages,12,7,1,360,50)==13);
    assert(pageSelectableRow(groupedPages,13,8,1,360,50)==13);
    assert(pageSelectableRow(rows,2,1,1,360,50)==5); // Empty console skipped.
    assert(pageSelectableRow(rows,5,4,-1,360,50)==1);
    assert(browserRowHeight(groupedPages[0],49)==49);
    assert(browserRowHeight(groupedPages[1],49)==60);
    std::cout<<"Existing row/console navigation: grouped/flat/empty/single/boundary/header-skipping behavior: PASS\n";
}
