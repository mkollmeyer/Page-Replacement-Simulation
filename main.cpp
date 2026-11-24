#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <linux/ptrace.h>

#include "replacement.h"

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    bool q = false;
    bool verbose = false;
    std::string policy;
    int maxPages = 10;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-q") {
            q = true;
        } 
        else if (args[i] == "-p") {
            if (i + 1 < args.size()) {
                policy = args[i + 1];
                ++i;
            } else {
                std::cerr << "Error: --p requires a policy.\n";
                return 1;
            }
        } else if (args[i] == "-n") {
            if (i + 1 < args.size()) {
                std::size_t parsedLength = 0;
                try {
                    maxPages = std::stoi(args[i + 1], &parsedLength);
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error: -n requires a positive integer page allocation.\n";
                    return 1;
                } catch (const std::out_of_range&) {
                    std::cerr << "Error: -n allocation is out of range.\n";
                    return 1;
                }
                if (parsedLength != args[i + 1].size() || maxPages <= 0) {
                    std::cerr << "Error: -n requires a positive integer page allocation.\n";
                    return 1;
                }
                ++i;
            } else {
                std::cerr << "Error: -n requires a page allocation.\n";
                return 1;
            }
        } else if (args[i] == "-v") {
            verbose = true;
        } 
    }
    if (policy != "fifo" && policy != "lru" && policy != "lfu" && policy != "clock") {
        std::cerr << "Error: select a policy with -p fifo|lru|lfu|clock.\n";
        return 1;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    pid_t ch_pid = fork();
    if (ch_pid == 0) {
        ptrace(static_cast<__ptrace_request>(PTRACE_TRACEME), getpid(), nullptr, nullptr);
        raise(SIGSTOP);
        char *args[] = {"./short-loop", NULL};
        execvp(args[0], args);
    } else {
        struct user_regs_struct regs;
        int status;
        int page = 0;
        waitpid(ch_pid, &status, 0);
        perror("After first wait in parent");
        ptrace((__ptrace_request) PTRACE_SETOPTIONS, ch_pid, nullptr, PTRACE_O_TRACESYSGOOD);
        perror("After PTRACE_SETOPTIONS");
        ptrace((__ptrace_request) PTRACE_SINGLESTEP, ch_pid, nullptr, nullptr);
        perror("After PTRACE_SYSCALL");
        replacement simulate(maxPages);
        while(true) {
            errno = 0;
            pid_t who = wait(&status);
            if (errno)
                perror("After parent wait in loop");
            if (who == -1 || errno || WIFEXITED(status) || WIFSIGNALED(status)) break;
            ptrace((__ptrace_request) PTRACE_GETREGS, ch_pid, nullptr, &regs);
            if (errno)
                perror("After GETREGS");
            page = regs.rip / page_size;
            if (policy == "fifo") {
                simulate.fifoAddPage(page);
            } else if (policy == "lru") {
                simulate.lruAddPage(page);
            } else if (policy == "lfu") {
                simulate.lfuAddPage(page);
            } else if (policy == "clock") {
                simulate.clockAddPage(page);
            }
            if (!q) {
                simulate.print(page, regs.rip, verbose);
            }
            int sig = (WSTOPSIG(status) == SIGTRAP) ? 0 : WSTOPSIG(status);
            ptrace((__ptrace_request) PTRACE_SINGLESTEP, ch_pid, nullptr, reinterpret_cast<void*>(sig));
        }
        simulate.summary();
    }
  return 0;
}
