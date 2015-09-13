#define EBSP_MSG_ORDERED(fmt, ...)\
    for(int i = 0; i < bsp_nprocs(); ++i) {\
        if(i == bsp_pid())\
            ebsp_message(fmt, __VA_ARGS__);\
        ebsp_barrier();\
    }
