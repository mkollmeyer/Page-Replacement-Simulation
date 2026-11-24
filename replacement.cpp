#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <iomanip>
#include "replacement.h"

replacement::replacement(int pageNumber){
  maxPages = pageNumber;
  pageFault = 0;
  ref = 0;
  ref2 = 0;
  lastPage = 0;
  hand = 0;
  fault = false;
}
void replacement::fifoAddPage(int addPage){
  ref++;
  fault = false;
  if (std::find(page.begin(), page.end(), addPage) == page.end()) {
    if (ref2 >= maxPages) {
      ref2++;
      page.erase(page.begin());
      page.push_back(addPage);
      pageFault++;
      fault = true;
    } else {
      ref2++;
      page.push_back(addPage);
    }
  }
}
void replacement::lfuAddPage(int addPage){
  ref++;
  auto it = pages.find(addPage);
  if (it != pages.end()) {
    pages[addPage] += 1;
    fault = false;
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
      fault = true;
    } else {
      pages.insert({addPage, 1});
      fault = false;
    }
  }
}
void replacement::lruAddPage(int addPage){
  ref++;
  auto it = pages.find(addPage);
  if (it != pages.end()) {
    for (auto& [key, val] : pages) {
        val -= 1; 
    }
    pages[addPage] = maxPages;
    fault = false;
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
      fault = true;
    } else {
      for (auto& [key, val] : pages) {
        val -= 1; 
      }
      pages.insert({addPage, maxPages});
      fault = false;
    }
  }
}
void replacement::clockAddPage(int addPage){
  ref++;
  auto it = pages.find(addPage);
  if (it != pages.end()) {
    pages[addPage] = 1;
    fault = false;
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
      fault = true;
    } else {
      pages.insert({addPage, 0});
      fault = false;
    }
  }
}
void replacement::print(unsigned long long int passPage, unsigned long long int rip, bool v) {
  if (lastPage != passPage) {
    lastPage = passPage;
    std::printf("[%d/%d] At: %llx Page: %llx\n", ref, pageFault, rip, lastPage);
  } else {
    if (v) {
      std::printf("[%d/%d] At: %llx\n", ref, pageFault, rip);
    }
  }
}
void replacement::printFault(unsigned long long int passPage, unsigned long long int rip) {
    if (fault) {
        std::printf("Page Fault #%d At: %llx\n", pageFault, rip);
    }
}
void replacement::summary() {
    double calc = ref == 0 ? 0.0 : static_cast<double>(pageFault) / ref;
    std::cout << "Page Allocation: " << maxPages << ", RefCount: " << ref
     << ", Page Faults: " << pageFault <<"(" << std::fixed << std::setprecision(4) << calc << ")\n";
}