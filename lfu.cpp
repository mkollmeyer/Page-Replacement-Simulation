#include <iostream>
#include <map>
#include <algorithm>
#include <unistd.h>
#include "lfu.h"

lfu::lfu(int pageNumber){
  std::map<int, int> pages;
  maxPages = pageNumber;
  pageFault = 0;
  ref = 0;
  ref2 = 0;
  page = 0;
}

void lfu::addPage(int addPage){
  ref++;
  auto it = pages.find(addPage);
  if (it != pages.end()) {
    pages[addPage] += 1;
  } else {
    ref2++;
    if (ref2 >= maxPages) {
      auto minIt = std::min_element(
        pages.begin(), 
        pages.end(),
        [](const auto& a, const auto& b) {
          return a.second < b.second;
        }
      );
      pages.erase(minIt);
      pages[addPage] = 1;
      pageFault++;
    } else {
      pages.insert({addPage, 1});
    }
  }
}
void lfu::print(unsigned long long int passPage, unsigned long long int rip) {
  if (page != passPage) {
    page = passPage;
    std::printf("[%d/%d] At: %llx Page: %llx\n", ref, pageFault, rip, page);
  } else {
    std::printf("[%d/%d] At: %llx\n", ref, pageFault, rip);
  }
}
