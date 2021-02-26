#ifndef MCM_CLIENT_H
#define MCM_CLIENT_H

typedef enum {
    MCM_STATUS_SUCCESS = 0,
    MCM_STATUS_FAIL_1 = 1,
    MCM_STATUS_TOTAL
} mcm_error_t;

typedef unsigned short uint16;
typedef unsigned int uint32;

typedef void (*mcm_client_ind_cb)
(
    uint32 hndl,
    void *ind_c_struct,
    uint32 ind_len
);

typedef struct {
    uint32 mcm_handle;
    mcm_client_ind_cb ind_cb;
} mcm_client_table_t;

uint32 mcm_client_init(uint32 *mcm_client_handle, mcm_client_ind_cb ind_cb);


#endif /*MCM_CLIENT_H*/