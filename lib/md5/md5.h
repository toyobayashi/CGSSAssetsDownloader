#ifndef __MD5_H__
#define __MD5_H__

#ifdef __cplusplus
extern "C" {
#endif

void md5(const uint8_t* initial_msg, size_t initial_len, uint8_t* digest);

#ifdef __cplusplus
}
#endif

#endif
