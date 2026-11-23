#include <iostream>
#include <map>
#include <algorithm>
#include <unistd.h>
#include "lru.h"

lru::lru(int pageNumber){
  std::map<int, int> pages;
  maxPages = pageNumber;
  pageFault = 0;
  ref = 0;
  ref2 = 0;
  page = 0;
}

void lru::addPage(int addPage){
  ref++;
  auto it = pages.find(addPage);
  if (it != pages.end()) {
    for (auto& [key, val] : pages) {
        val -= 1; 
    }
    pages[addPage] = maxPages;
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
      for (auto& [key, val] : pages) {
        val -= 1; 
      }
      pages[addPage] = maxPages;
      pageFault++;
    } else {
      for (auto& [key, val] : pages) {
        val -= 1; 
      }
      pages.insert({addPage, maxPages});
    }
  }
}
void lru::print(unsigned long long int passPage, unsigned long long int rip) {
  if (page != passPage) {
    page = passPage;
    std::printf("[%d/%d] At: %llx Page: %llx\n", ref, pageFault, rip, page);
  } else {
    std::printf("[%d/%d] At: %llx\n", ref, pageFault, rip);
  }
}
