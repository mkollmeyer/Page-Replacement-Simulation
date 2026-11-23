#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <linux/ptrace.h>
#include "FIFO.h"

int main(int argc, char* argv[]) {
    long page_size = sysconf(_SC_PAGESIZE);
    pid_t ch_pid = fork();
    if (ch_pid == 0) {
        // Tell parent to trace this process
        ptrace(static_cast<__ptrace_request>(PTRACE_TRACEME), getpid(), nullptr, nullptr);
        raise(SIGSTOP); // stop myself so the parent can start tracing
        char *args[] = {"./short-loop", NULL};
        execvp(args[0], args);
    } else {
        struct user_regs_struct regs;
        int status;
        int page = 0;
        // Wait for the child to stop before starting to trace its system calls
        waitpid(ch_pid, &status, 0);
        perror("After first wait in parent");
        ptrace((__ptrace_request) PTRACE_SETOPTIONS, ch_pid, nullptr, PTRACE_O_TRACESYSGOOD);
        perror("After PTRACE_SETOPTIONS");
        // Start tracing at the (first) system call of the child process
        ptrace((__ptrace_request) PTRACE_SINGLESTEP, ch_pid, nullptr, nullptr);
        perror("After PTRACE_SYSCALL");
        FIFO simulate(20);
        while(true) {
            errno = 0;
            pid_t who = wait(&status);
            if (errno)
                perror("After parent wait in loop");
            if (who == -1 || errno || WIFEXITED(status) || WIFSIGNALED(status)) break;
            // Get register information about the current instruction
            ptrace((__ptrace_request) PTRACE_GETREGS, ch_pid, nullptr, &regs);
            if (errno)
                perror("After GETREGS");
            page = regs.rip / page_size;
            simulate.addPage(regs.rip);
            simulate.print(page, regs.rip);
            // Continue tracing to the next instruction of the child process
            int sig = (WSTOPSIG(status) == SIGTRAP) ? 0 : WSTOPSIG(status);
            ptrace((__ptrace_request) PTRACE_SINGLESTEP, ch_pid, nullptr, reinterpret_cast<void*>(sig));
        }
        std::cout << "From Parent" << std::endl;
    }
  return 0;
}
