
#ifndef _XM_AUTH_H_
#define _XM_AUTH_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>

#define AUTH_SUCCESS 0
#define AUTH_FAIL 1

#define CURRENT_COMMUNICATE_SPP 0
#define CURRENT_COMMUNICATE_BLE 1


int get_random_auth_data(unsigned char* result);

//�޸�Ĭ��key��������Ĭ��ֵ����ʱ�������޸�
int set_link_key(const unsigned char* key);

int get_encrypted_auth_data(const unsigned char* random, unsigned char* result);

//libc ���� Ĭ�ϵ���libc����,ƽ̨����ʵ������滻
void * __attribute__((weak)) xm_malloc(size_t __size);

void  __attribute__((weak)) xm_free(void *);

int __attribute__((weak)) xm_rand(void);
//xm_srand�ڲ�Ĭ�����Ӻ���Ϊtime() ����ʵ�����,���滻srand
void  __attribute__((weak)) xm_srand();

void * __attribute__((weak)) xm_memcpy(void *str1, const void *str2, size_t n);

void * __attribute__((weak)) xm_memset(void *str, int c, size_t n);

int __attribute__((weak)) xm_log(const char *format, ...);


#ifdef __cplusplus
}
#endif

#endif	/* _XM_AUTH_H_ */

