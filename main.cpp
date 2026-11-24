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
    bool q = false;
    bool verbose = false;
    std::string policy;
    std::vector<char*> execArgs;
    int maxPages = 10;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-q") {
            q = true;
        } else if (arg == "-p") {
            if (i + 1 < argc) {
                policy = argv[++i];
            } else {
                std::cerr << "Error: --p requires a policy.\n";
                return 1;
            }
        } else if (arg == "-n") {
            if (i + 1 < argc) {
                std::string allocation = argv[++i];
                std::size_t parsedLength = 0;
                try {
                    maxPages = std::stoi(allocation, &parsedLength);
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error: -n requires a positive integer page allocation.\n";
                    return 1;
                } catch (const std::out_of_range&) {
                    std::cerr << "Error: -n allocation is out of range.\n";
                    return 1;
                }
                if (parsedLength != allocation.size() || maxPages <= 0) {
                    std::cerr << "Error: -n requires a positive integer page allocation.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: -n requires a page allocation.\n";
                return 1;
            }
        } else if (arg == "-v") {
            verbose = true;
        } else if (arg == "--") {
            for (int j = i + 1; j < argc; ++j) {
                execArgs.push_back(argv[j]);
            }
            break;
        } else if (!arg.empty() && arg[0] != '-') {
            for (int j = i; j < argc; ++j) {
                execArgs.push_back(argv[j]);
            }
            break;
        } else {
            std::cerr << "Error: unknown option " << arg << ".\n";
            return 1;
        }
    }
    char defaultProgram[] = "./short-loop";
    if (execArgs.empty()) {
        execArgs.push_back(defaultProgram);
    }
    execArgs.push_back(nullptr);
    if (policy != "fifo" && policy != "lru" && policy != "lfu" && policy != "clock") {
        std::cerr << "Error: select a policy with -p fifo|lru|lfu|clock.\n";
        return 1;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    pid_t ch_pid = fork();
    if (ch_pid == 0) {
        ptrace(static_cast<__ptrace_request>(PTRACE_TRACEME), getpid(), nullptr, nullptr);
        raise(SIGSTOP);
        execvp(execArgs[0], execArgs.data());
        perror("execvp");
        _exit(127);
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
            } else {
                simulate.printFault(page, regs.rip);
            }
            int sig = (WSTOPSIG(status) == SIGTRAP) ? 0 : WSTOPSIG(status);
            ptrace((__ptrace_request) PTRACE_SINGLESTEP, ch_pid, nullptr, reinterpret_cast<void*>(sig));
        }
        simulate.summary();
    }
  return 0;
}
