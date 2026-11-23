#ifndef CLOCK_H
#define CLOCK_H

#include <map>
#include <algorithm>

class Clock {
  private:
    std::map<int, int> pages;
    int maxPages;
    int pageFault;
    int ref;
    int ref2;
    unsigned long long int page;
    int hand;

  public:
    Clock(int pageNumber);
    void addPage(int addPage);
    void print(unsigned long long int passPage, unsigned long long int rip);
};

#endif
