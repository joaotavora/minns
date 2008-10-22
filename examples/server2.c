/* Multithreaded HTTP Server for Linux built up by the pool of threads
 * multiplexing select() system call.
 * Developing code bellow we follow general guidelines from [1] chapter 11.
 * The only one source file has been modified is server.c.
 * Instead of fork new process to handle each incoming client's connection,
 * as procedure server_run ( ) does in [1], we start pool of 1024 threads
 * accepting as parameter descriptor of passive socket.
 * The procedure run by each thread is asynchronous BSD socket's server
 * utilizing
 * select() system call (see for example [2],chapter 13(5)) to switch between
 * handling
 * incoming client's requests by accept() system call and receiving "http"
 * requests
 * from clients already connected to server utilizing procedures
 * handle_request() and handle_get() from [1],chapter 11.
 * References
 * 1.Mark Mitchel,Jeffrey Oldham,Alex Samuel. Advanced Linux Programming.
 * New Riders Publishing , 2001. www.advancedlinuxprogramming.com
 * 2.Douglas E. Comer,David L. Stevens Internet Working with TCP/IP ,vol 3
 * Client-Server Programming and application Linux/Posix Socket Version,
 * Prentice Hall,Inc. 2001 */
/*
 * server.c module :- multithreaded version, starting pool of threads,
 * multiplexing select() system call, to serve new connections
 * utilizing accept() system call and "http" responds for clients
 * already connected to server simultaneously.
 */
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"
#define NUM_THREADS 1024
pthread_mutex_t get_mutex=PTHREAD_MUTEX_INITIALIZER;
static char* not_found_response_template =
    "HTTP/1.0 404 Not Found\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    " <h1>Not Found</h1>\n"
    " <p>The requested URL %s was not found on this server.</p>\n"
    " </body>\n"
    "</html>\n";
static char* bad_method_response_template =
    "HTTP/1.0 501 Method Not Implemented\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    " <h1>Method Not Implemented</h1>\n"
    " <p>The method %s is not implemented by this server.</p>\n"
    " </body>\n"
    "</html>\n";
static char* ok_response =
    "HTTP/1.0 200 OK\n"
    "Content-type: text/html\n"
    "\n";
static char* bad_request_response =
    "HTTP/1.0 400 Bad Request\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    " <h1>Bad Request</h1>\n"
    " <p>This server did not understand your request.</p>\n"
    " </body>\n"
    "</html>\n";
void
die(const char *func, int err)
{
    fprintf(stderr,"%s: %s\n",func,strerror(err));
    abort();
}
static void handle_get (int connection_fd, const char* page)
{
    struct server_module* module = NULL;
    if (*page == '/' && strchr (page + 1, '/') == NULL) {
        char module_file_name[64];
        snprintf (module_file_name, sizeof (module_file_name),
            "%s.so", page + 1);
        module = module_open (module_file_name);
    }
    if (module == NULL) {
        char response[1024];
        snprintf (response, sizeof (response), not_found_response_template,
            page);
        write (connection_fd, response, strlen (response));
    }
    else {
        write (connection_fd, ok_response, strlen (ok_response));
        (*module->generate_function) (connection_fd);
        module_close (module);
    }
}
static void *
handle_connection (int fdSock)
{
    int connection_fd = fdSock;
    char buffer[256];
    ssize_t bytes_read;
    bytes_read = read (connection_fd, buffer, sizeof (buffer) - 1);
    if (bytes_read > 0) {
        char method[sizeof (buffer)];
        char url[sizeof (buffer)];
        char protocol[sizeof (buffer)];
        buffer[bytes_read] = '\0';
        sscanf (buffer, "%s %s %s", method, url, protocol);
        while (strstr (buffer, "\r\n\r\n") == NULL)
            bytes_read = read (connection_fd, buffer, sizeof (buffer));
        if (bytes_read == -1) {
            close (connection_fd);
            return;
        }
        if (strcmp (protocol, "HTTP/1.0") && strcmp (protocol, "HTTP/1.1")) {
            write (connection_fd, bad_request_response,
                sizeof (bad_request_response));
        }
        else if (strcmp (method, "GET")) {
            char response[1024];
            snprintf (response, sizeof (response),
                bad_method_response_template, method);
            write (connection_fd, response, strlen (response));
        }
        else
            handle_get (connection_fd, url);
    }
    else if (bytes_read == 0)
        ;
    else {
        system_error ("read");
    }
    close(connection_fd);
}
void *
serv_request(void *insock)
{
    int msock = (int)insock;
    struct sockaddr_in fsin;
    fd_set rfds;
    fd_set afds;
    unsigned int alen;
    int fd,nfds;
    int rval;
    socklen_t address_length;
    struct sockaddr_in socket_address;
    nfds=getdtablesize();
    FD_ZERO(&afds);
    FD_SET(msock,&afds);
    while(1) {
        memcpy(&rfds,&afds,sizeof(rfds));
        if (select(nfds,&rfds,(fd_set *)0,(fd_set *)0,(struct timeval *)0) < 0)
            die("select()",errno);
        if (FD_ISSET(msock,&rfds)) {
            int ssock;
            alen=sizeof(fsin);
/*
  Serving new incoming client's connection
*/
            pthread_mutex_lock(&get_mutex);
            ssock=accept(msock,(struct sockaddr *)&fsin,&alen);
            pthread_mutex_unlock(&get_mutex);
            if (ssock < 0)
                die ("accept()",errno);
            if (verbose) {
                socklen_t address_length;
                address_length = sizeof (socket_address);
                rval = getpeername (ssock, &socket_address, &address_length);
                assert (rval == 0);
                printf ("Thread '%d' accepted connection from %s\n",
                    pthread_self(),inet_ntoa (socket_address.sin_addr));
            }
            FD_SET(ssock,&afds);
        }
        for (fd=0;fd < nfds; ++fd)
            if (fd != msock && FD_ISSET(fd,&rfds)) {
                printf("Thread '%d' responding request...\n",pthread_self());
/*
  Serving client had been already connected to server
*/
                handle_connection(fd);
                FD_CLR(fd,&afds);
            }
    }
}
void
server_run(struct in_addr local_address,uint16_t port)
{
    struct sockaddr_in socket_address;
    int rval;
    int server_socket;
    int k;
/*
  Getting socket's descriptor to be passed to
  each thread in the pool
*/
    server_socket = socket (PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        system_error ("socket");
    memset (&socket_address, 0, sizeof (socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = port;
    socket_address.sin_addr = local_address;
    rval = bind (server_socket, &socket_address, sizeof (socket_address));
    if (rval != 0)
        system_error ("bind");
    rval = listen (server_socket, 10);
    if (rval != 0)
        system_error ("listen");
    if (verbose) {
        socklen_t address_length;
        address_length = sizeof (socket_address);
        rval = getsockname (server_socket, &socket_address, &address_length);
        assert (rval == 0);
        printf ("server listening on %s:%d\n",
            inet_ntoa (socket_address.sin_addr),
            (int) ntohs (socket_address.sin_port));
    }
/*
  Starting pool of threads building up HTTP server
*/
    size_t stacksize;
    pthread_t p_thread[NUM_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    stacksize = 500000;
    pthread_attr_setstacksize (&attr, stacksize);
    pthread_attr_getstacksize (&attr, &stacksize);
    for(k=0; k<NUM_THREADS; k++) {
        pthread_create(&p_thread[k],&attr,serv_request,(void*)server_socket);
    }
    pthread_attr_destroy(&attr);
    for(k=0;k<NUM_THREADS;k++) {
        pthread_join(p_thread[k], NULL);
        printf("Completed join with thread %d\n",k);
    }
}

