#ifndef __METAPATH_DATA_STRUCT__
#define __METAPATH_DATA_STRUCT__

typedef struct __attribute__((packed)){
    uint16_t state;
} metapath_sm_lut_t;


typedef struct __attribute__((packed)){
    uint32_t updated_state;
} metapath_state_t;


#endif /* __METAPATH_DATA_STRUCT__ */
