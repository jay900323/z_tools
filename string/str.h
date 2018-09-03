#ifndef _Z_STR_H
#define _Z_STR_H


typedef unsigned int size_t;

/** @see isalnum */
#define z_isalnum(c) (isalnum(((unsigned char)(c))))
/** @see isalpha */
#define z_isalpha(c) (isalpha(((unsigned char)(c))))
/** @see iscntrl */
#define z_iscntrl(c) (iscntrl(((unsigned char)(c))))
/** @see isdigit */
#define z_isdigit(c) (isdigit(((unsigned char)(c))))
/** @see isgraph */
#define z_isgraph(c) (isgraph(((unsigned char)(c))))
/** @see islower*/
#define zr_islower(c) (islower(((unsigned char)(c))))
/** @see isascii */
#ifdef isascii
#define z_isascii(c) (isascii(((unsigned char)(c))))
#else
#define z_isascii(c) (((c) & ~0x7f)==0)
#endif

/** @see isprint */
#define z_isprint(c) (isprint(((unsigned char)(c))))
/** @see ispunct */
#define z_ispunct(c) (ispunct(((unsigned char)(c))))
/** @see isspace */
#define z_isspace(c) (isspace(((unsigned char)(c))))
/** @see isupper */
#define z_isupper(c) (isupper(((unsigned char)(c))))
/** @see isxdigit */
#define z_isxdigit(c) (isxdigit(((unsigned char)(c))))
/** @see tolower */
#define z_tolower(c) (tolower(((unsigned char)(c))))
/** @see toupper */
#define z_toupper(c) (toupper(((unsigned char)(c))))

#include <stdio.h>
/*
 * ZKIT用于取代strncpy函数，针对strncpy做了如下改动
 *   (1) strncpy碰到空字符结束后如果拷贝长度不够会用\0填充
 *   (2) 返回的dst字符串会以\0结束
 *   (3) 如果src为NULL直接返回dst

 *  参数说明：
 *      dest: 目标缓冲区首地址
 *      src:  源字符串首地址
 *	    dst_size: 目标缓冲区大小
 *      返回值：目标缓冲区地址
*/
char * z_cpystrn(char *dst, const char *src, size_t  dst_size);

char *z_strstr(char *s1, char *s2);

/******************************************************************************
*                                                                            *
* Function: z_strlcat                                                      *
*                                                                            *
* Purpose: Appends src to string dst of size siz (unlike strncat, dst_size is    *
*          the full size of dst, not space left). At most siz - 1 characters *
*          will be copied. Always null terminates (unless                    *
*          siz <= strlen(dst)).                                              *
*                                                                            *
******************************************************************************/
void	z_strlcat(char *dst, const char *src, size_t dst_size)'

int z_strcasecmp(const char *a, const char *b);

int z_strncasecmp(const char *a, const char *b, size_t n);

char *z_nextline(char **buf);

int z_strendswith(char *dest, const char *sep);

#endif
