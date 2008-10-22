// Some constants and useful functions common to all the program
// TODO: check if `restrict' keyword is accepted.

const int maxbuff = 256;

// Error printing functions. From W. Richard Stevens' "Unix Network
// Programming: Volume 1, Second Edition"
void    err_dump(const char *, ...);
void    err_msg(const char *, ...);
void    err_quit(const char *, ...);
void    err_ret(const char *, ...);
void    err_sys(const char *, ...);

// wrappers to read() and write() system calls
void    readline_w(int sockfd, char *line, const int max);
int     writen_w(int sockfd, char *line, const int max);

// wrappers to accept(), bind() and listen() system calls
int     accept_w(int socket, struct sockaddr *address, socklen_t * address_len);
int     bind_w(int socket, const struct sockaddr *address, socklen_t address_len);
int     listen_w(int listenfd, int backlog);

// wrappers to pthread_* functions
int     pthread_mutex_lock_w(pthread_mutex_t *mutex);
int     pthread_mutex_unlock_w(pthread_mutex_t *mutex);
int     pthread_create_w(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine)(void *), void * arg);



