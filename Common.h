// Some constants and useful functions common to all the program

const int maxbuff = 256;

// Error printing functions. From W. Richard Stevens' "Unix Network
// Programming: Volume 1, Second Edition"
void    err_dump(const char *, ...);    /* {App misc_source} */
void    err_msg(const char *, ...);
void    err_quit(const char *, ...);
void    err_ret(const char *, ...);
void    err_sys(const char *, ...);
