#include "Common.h"
#include "EchoHandler.h"

void EchoHandler::handle(int sockfd){
    int                 ntowrite;
    ssize_t             nread;
    char                line[MAXLINE], result[MAXN];

    for ( ; ; ) {
        if ( (nread = readline_w(sockfd, line, MAXLINE)) == 0)
            return;             /* connection closed by other end */

        /* 4line from client specifies #bytes to write back */
        ntowrite = atol(line);
        if ((ntowrite <= 0) || (ntowrite > MAXN))
            err_quit("client request for %d bytes", ntowrite);

        writen_w(sockfd, result, ntowrite);
    }
}
