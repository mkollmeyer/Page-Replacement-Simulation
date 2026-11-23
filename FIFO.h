#ifndef FIFO_H
#define FIFO_H

#include <vector>

class FIFO {
  private:
    std::vector<int> pages;
    int maxPages;
    int pageFault;
    int ref;
    unsigned long long int page;

  public:
    FIFO(int pageNumber);
    void addPage(int addPage);
    void print(unsigned long long int passPage, unsigned long long int rip);
};

#endif
