#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <linux/ptrace.h>

int main(int argc, char* argv[]) {
    pid_t ch_pid = fork();
    if (ch_pid == 0) {
        // Tell parent to trace this process
        ptrace(static_cast<__ptrace_request>(PTRACE_TRACEME), getpid(), nullptr, nullptr);
        raise(SIGSTOP); // stop myself so the parent can start tracing
        char *args[] = {(char *) "ls", (char *) "-l", nullptr};
        execvp(args[0], args);
    } else {
        ptrace_syscall_info sc_info;
        // Wait for the child to stop before starting to trace its system calls
        wait(nullptr);
        perror("After first wait in parent");
        ptrace((__ptrace_request) PTRACE_SETOPTIONS, ch_pid, nullptr, PTRACE_O_TRACESYSGOOD);
        perror("After PTRACE_SETOPTIONS");
        // Start tracing at the (first) system call of the child process
        ptrace((__ptrace_request) PTRACE_SYSCALL, ch_pid, nullptr, nullptr);
        perror("After PTRACE_SYSCALL");
        while(true) {
            pid_t who = wait(nullptr);
            if (errno)
                perror("After parent wait in loop");
            if (who == -1 || errno) break;
            // Get information about the current system call
            ptrace((__ptrace_request) PTRACE_GET_SYSCALL_INFO, ch_pid, sizeof(sc_info), &sc_info);
            if (errno)
                perror("After GET_SYSCALL_INFO");
            switch(sc_info.op) {
                case PTRACE_SYSCALL_INFO_ENTRY:
                    // This point is reached when the child is about to start a system call
                    std::printf("System call ID:%lld ", sc_info.entry.nr);
                    break;
                case PTRACE_SYSCALL_INFO_EXIT:
                    // This point is reached when the child has completed the system call
                    if (!sc_info.exit.is_error)
                        std::printf("==> %llx\n", sc_info.exit.rval);
                    else
                        std::printf("==> ERROR\n");
                    break;
                case PTRACE_SYSCALL_INFO_NONE:
                    std::printf("None of options???\n");
                    break;
                default:
                    std::printf("Unknown action ?????\n");
            }
            // Continue tracing to the next system call of the child process
            ptrace((__ptrace_request) PTRACE_SYSCALL, ch_pid, nullptr, nullptr);
        }
        std::cout << "From Parent" << std::endl;
    }
    return 0;
}

