#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include <map>
#include <vector>
#include <algorithm>

class replacement {
  private:
    std::vector<int> page;
    std::map<int, int> pages;
    int maxPages;
    int pageFault;
    int ref;
    int ref2;
    unsigned long long int lastPage;
    int hand;
    bool fault;

  public:
    replacement(int pageNumber);
    void fifoAddPage(int addPage);
    void lruAddPage(int addPage);
    void lfuAddPage(int addPage);
    void clockAddPage(int addPage);
    void print(unsigned long long int passPage, unsigned long long int rip, bool v);
    void printFault(unsigned long long int passPage, unsigned long long int rip);
    void summary();
};

#endif
