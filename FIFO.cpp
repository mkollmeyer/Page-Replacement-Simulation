#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include "FIFO.h"

FIFO::FIFO(int pageNumber){
  std::vector<int> pages;
  maxPages = pageNumber;
  pageFault = 0;
  ref = 0;
  page = 0;
}

void FIFO::addPage(int addPage){
  ref++;
  if (std::find(pages.begin(), pages.end(), addPage) == pages.end()) {
    if (ref >= maxPages) {
      pages.erase(pages.begin());
      pages.push_back(addPage);
      pageFault++;
    } else {
      pages.push_back(addPage);
    }
  }
}
void FIFO::print(unsigned long long int passPage, unsigned long long int rip) {
  if (page != passPage) {
    page = passPage;
    std::printf("[%d/%d] At: %llx Page: %llx\n", ref, pageFault, rip, page);
  } else {
    std::printf("[%d/%d] At: %llx\n", ref, pageFault, rip);
  }
}
