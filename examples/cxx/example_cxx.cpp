#include <arpa/inet.h>
#include <co.h>
#include <coxx.h>
#include <dbg/dbgmsg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strerror
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

void connection(int fd) {
    printf("new incoming fd(%d) accepted.\n", fd);
    char buf[1024];
    // Set to non-blocking mode.
    int flags = fcntl(fd, F_GETFL, 0);
    if (!(flags & O_NONBLOCK))
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    // Waiting for data.
retry_read:
    co::yield();
    int n = read(fd, buf, sizeof(buf));
    if (n == -1) {
        if (EAGAIN == errno || EINTR == errno)
            goto retry_read;
        fprintf(stderr, "read error:%s\n", strerror(errno));
        goto exit;
    } else if (n == 0) {
        fprintf(stderr, "read eof\n");
        goto exit;
    }
    // Echo
    co::yield();
    write(fd, buf, n);
exit:
    co::yield();
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

void listener() {
    printf("listening on 127.0.0.1:1234...\n");
    // Creates a server socket listen on 127.0.0.1:1234
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len = sizeof(addr);
    if (-1 == bind(fd, (sockaddr *) &addr, len)) {
        fprintf(stderr, "bind error, please change the port value.\n");
        return;
    }
    if (-1 == listen(fd, 5)) {
        fprintf(stderr, "listen error.\n");
        return;
    }
    // Set to non-blocking mode.
    int flags = fcntl(fd, F_GETFL, 0);
    if (!(flags & O_NONBLOCK))
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    // Waiting for incoming connections.
    for (;;) {
        co::yield();
        int sockFd = accept(fd, (sockaddr *) &addr, &len);
        if (sockFd == -1) {
            if (EAGAIN == errno || EINTR == errno)
                continue;
            fprintf(stderr, "accept error:%s\n", strerror(errno));
            return;
        }
        co::go([sockFd] { connection(sockFd); });
    }
}

int main(int argc, char *argv[]) {
    dbg_log_level(DLI_WARN);
    co::sched sched;
    sched.go(listener);
    sched.runloop();
    return 0;
}
