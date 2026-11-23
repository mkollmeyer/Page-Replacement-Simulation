#include <iostream>
#include <map>
#include <algorithm>
#include <unistd.h>
#include "clock.h"

Clock::Clock(int pageNumber){
  std::map<int, int> pages;
  maxPages = pageNumber;
  pageFault = 0;
  ref = 0;
  ref2 = 0;
  page = 0;
  hand = 0;
}

void Clock::addPage(int addPage){
  ref++;
  auto it = pages.find(addPage);
  if (it != pages.end()) {
    pages[addPage] = 1;
  } else {
    ref2++;
    if (ref2 >= maxPages) {
      bool none_kicked = true;
      while (none_kicked) {
        auto it = std::next(pages.begin(), hand);
        if (it->second == 1) {
          pages[it->first] = 0;
        }  else {
          pages.erase(it->first);
          pages[addPage] = 0;
          none_kicked = false;
        }
        hand = (hand + 1) % maxPages;
      }
      pageFault++;
    } else {
      pages.insert({addPage, 0});
    }
  }
}
void Clock::print(unsigned long long int passPage, unsigned long long int rip) {
  if (page != passPage) {
    page = passPage;
    std::printf("[%d/%d] At: %llx Page: %llx\n", ref, pageFault, rip, page);
  } else {
    std::printf("[%d/%d] At: %llx\n", ref, pageFault, rip);
  }
}
