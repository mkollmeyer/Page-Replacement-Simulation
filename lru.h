#ifndef lru_H
#define lru_H

#include <map>
#include <algorithm>

class lru {
  private:
    std::map<int, int> pages;
    int maxPages;
    int pageFault;
    int ref;
    int ref2;
    unsigned long long int page;

  public:
    lru(int pageNumber);
    void addPage(int addPage);
    void print(unsigned long long int passPage, unsigned long long int rip);
};

#endif
