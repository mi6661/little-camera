#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#if 0

int _write(int file, char *ptr, int len) {
    // 只处理 stdout / stderr
    if (file != STDOUT_FILENO && file != STDERR_FILENO) {
        errno = EBADF;
        return -1;
    }

    for (int i = 0; i < len; i++) {
        // 把你的 UART 发送函数放这里
        //uart_send_byte(ptr[i]);
    }
    return len;
}

// 通常还需要这些空实现，避免链接报错
int _close(int file) { return -1; }
int _fstat(int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
int _isatty(int file) { return 1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _read(int file, char *ptr, int len) { return 0; }
int _getpid(void) { return 1; }
int _kill(int pid, int sig) { errno = EINVAL; return -1; }

#endif