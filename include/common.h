#ifndef COMMON_H 
#define COMMON_H

#define IN
#define OUT

typedef __UINT64_TYPE__ uint64_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT8_TYPE__  uint8_t;
typedef __INT64_TYPE__ int64_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT8_TYPE__  int8_t;

typedef struct {
    uint8_t * array;
    uint64_t size; /* size in bytes, */
} array_t;
typedef struct {
    const uint8_t * str;
    uint64_t size; /* size in bytes, 
    the size must be equal to string's length including its null terminator */
} string_t, readonly_array_t;

inline
bool
COMMON_check_array_is_valid(
    IN const array_t & array
){
    if (nullptr == array.array) {
        /* @todo: fill error */
        return false;
    }
    return true;
}

/**
 * @brief checks that the recived string is valid
 * 
 * @note sets error code
 * @note has multiple returns because this should be very short and simple to read function
 * 
 * @param[in] string the string to check
 */
inline
bool
COMMON__check_string_is_valid(
    IN const string_t & string
){
    if (nullptr == string.str) {
        /* @todo: fill error */
        return false;
    }
    if (string.str[string.size - 1] != '\0'){
        /* @todo: fill error */
        return false;
    }
    return true;
}

#endif
