#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>  
#include <errno.h>

// 함수 원형 선언
void commandmenu(pid_t pid, char* command);
void ni(int pid);
void cont(int pid);

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ./swingdbg <target program>\n");
        return -1;
    }

    int pid = fork();
    char command[100];

    if (pid == 0) {
        // 자식 프로세스
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            perror("err: ptrace");
            return -1;
        }
        execl(argv[1], argv[1], (char *)NULL);
        perror("execl failed");  // 실패 시 출력
        return -1;
    } 
    else if (pid > 0) {
        // 부모 프로세스 (디버거)
        int stat;
        waitpid(pid, &stat, 0);

        while (WIFSTOPPED(stat)) {
            printf("SWINGdbg >>> ");
            if (fgets(command, sizeof(command), stdin) == NULL) {
                break;
            }
            command[strcspn(command, "\n")] = 0;  // 개행 제거

            commandmenu(pid, command);

            // 다음 명령 처리를 위해 상태 업데이트
            waitpid(pid, &stat, 0);
        }
    } 
    else {
        perror("err: fork");
        return -1;
    }

    return 0;
}

// 명령어 분기 처리
void commandmenu(pid_t pid, char* command) {
    if (strcmp(command, "ni") == 0) {
        ni(pid);
    } 
    else if (strcmp(command, "c") == 0) {
        cont(pid);
    } 
    else {
        printf("Unknown command: %s\n", command);
    }
}

// ni: 한 줄 실행 + 레지스터 상태 출력
void ni(int pid) {
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) != 0) {
        perror("ptrace(GETREGS)");
        return;
    }

    unsigned long instr = ptrace(PTRACE_PEEKTEXT, pid, (void*)regs.rip, NULL);
    if (instr == -1L && errno != 0) {
        perror("ptrace(PEEKTEXT)");
        return;
    }

    printf("[+] RIP = 0x%llx | instr = 0x%lx\n", regs.rip, instr);
    printf("[+] Registers:\n");
    printf("RAX: 0x%llx\n", regs.rax);
    printf("RBX: 0x%llx\n", regs.rbx);
    printf("RCX: 0x%llx\n", regs.rcx);
    printf("RDX: 0x%llx\n", regs.rdx);
    printf("RSI: 0x%llx\n", regs.rsi);
    printf("RDI: 0x%llx\n", regs.rdi);
    printf("RSP: 0x%llx\n", regs.rsp);
    printf("RBP: 0x%llx\n", regs.rbp);

    if (ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL) != 0) {
        perror("ptrace(SINGLESTEP)");
        return;
    }
}

// c: 계속 실행
void cont(int pid) {
    if (ptrace(PTRACE_CONT, pid, NULL, NULL) != 0) {
        perror("ptrace(CONT)");
        return;
    }
}