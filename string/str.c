#include "str.h"
#include "common.h"
#include <assert.h>

#define MIN(a, b) ((a<b)?a:b)

char * z_cpystrn(char *dst, const char *src, size_t dst_size)
{
	char *d = dst, *end;

	if (dst_size <= 0 ) {
		return (dst);
	}

	if (src) {
		end = dst + dst_size - 1;

		for (; d < end; ++d, ++src) {
			if (!(*d = *src)) {
				return (d);
			}
		}
	}

	*d = '\0';

	return (d);
}


char *z_strstr(char *s1, char *s2)
{
	char *p1, *p2;
	if (*s2 == '\0') {
		/* an empty s2 */
		return(s1);
	}
	while((s1 = strchr(s1, *s2)) != NULL) {
		/* found first character of s2, see if the rest matches */
		p1 = s1;
		p2 = s2;
		while (*++p1 == *++p2) {
			if (*p1 == '\0') {
				/* both strings ended together */
				return(s1);
			}
		}
		if (*p2 == '\0') {
			/* second string ended, a match */
			break;
		}
		if(*p1 == '\0'){
			return NULL;
		}
		/* didn't find a match here, try starting at next character in s1 */
		s1++;
	}
	return(s1);
}

void	z_strlcat(char *dst, const char *src, size_t dst_size)
{
	while ('\0' != *dst)
	{
		dst++;
		dst_size--;
	}

	z_cpystrn(dst, src, dst_size);
}

int z_strcasecmp(const char *a, const char *b)
{
	const char *p = a;
	const char *q = b;
	for (p = a, q = b; *p && *q; p++, q++) {
		int diff = z_tolower(*p) - z_tolower(*q);
		if (diff)
			return diff;
	}
	if (*p)
		return 1;               /* p was longer than q */
	if (*q)
		return -1;              /* p was shorter than q */
	return 0;                   /* Exact match */
}

int z_strncasecmp(const char *a, const char *b, size_t n)
{
	const char *p = a;
	const char *q = b;

	for (p = a, q = b; /*NOTHING */ ; p++, q++) {
		int diff;
		if (p == a + n)
			return 0;           /*   Match up to n characters */
		if (!(*p && *q))
			return *p - *q;
		diff = z_tolower(*p) - z_tolower(*q);
		if (diff)
			return diff;
	}
	/*NOTREACHED */
}

char * z_strtok(char *str, const char *sep, char **last)
{
	char *token;

	if (!str)           /* subsequent call */
		str = *last;    /* start where we left off */

	/* skip characters in sep (will terminate at '\0') */
	while (*str && strchr(sep, *str))
		++str;

	if (!*str)          /* no more tokens */
		return NULL;

	token = str;

	/* skip valid token characters to terminate token and
	* prepare for the next call (will terminate at '\0) 
	*/
	*last = token + 1;
	while (**last && !strchr(sep, **last))
		++*last;

	if (**last) {
		**last = '\0';
		++*last;
	}

	return token;
}

/* Filepath_name_get returns the final element of the pathname.
* Using the current platform's filename syntax.
*   "/foo/bar/gum" -> "gum"
*   "D:\foo\bar\gum.txt" -> "gum.txt"
*   "/foo/bar/gum/" -> ""
*   "gum" -> "gum"
*   "wi\\n32\\stuff" -> "stuff
*
* Corrected Win32 to accept "a/b\\stuff", "a:stuff"
*/

const char * z_filepath_name_get(const char *pathname)
{
	const char path_separator = '/';
	const char *s = strrchr(pathname, path_separator);

#ifdef WIN32
	const char path_separator_win = '\\';
	const char drive_separator_win = ':';
	const char *s2 = strrchr(pathname, path_separator_win);

	if (s2 > s) s = s2;

	if (!s) s = strrchr(pathname, drive_separator_win);
#endif

	return s ? ++s : pathname;

}


size_t	z_vsnprintf(char *str, size_t count, const char *fmt, va_list args)
{
	int	written_len = 0;

	if (0 < count)
	{
		if (0 > (written_len = vsnprintf(str, count, fmt, args)))
			written_len = (int)count - 1;		/* count an output error as a full buffer */
		else
			written_len = MIN(written_len, (int)count - 1);		/* result could be truncated */
	}
	str[written_len] = '\0';	/* always write '\0', even if buffer size is 0 or vsnprintf() error */

	return (size_t)written_len;
}

size_t	z_snprintf(char *str, size_t count, const char *fmt, ...)
{
	size_t	written_len;
	va_list	args;

	va_start(args, fmt);
	written_len = z_vsnprintf(str, count, fmt, args);
	va_end(args);

	return written_len;
}

char	*z_string_replace(const char *str, const char *sub_str1, const char *sub_str2)
{
	char *new_str = NULL;
	const char *p;
	const char *q;
	const char *r;
	char *t;
	long len;
	long diff;
	unsigned long count = 0;

	assert(str);
	assert(sub_str1);
	assert(sub_str2);

	len = (long)strlen(sub_str1);

	/* count the number of occurrences of sub_str1 */
	for ( p=str; (p = strstr(p, sub_str1)); p+=len, count++ );

	if (0 == count)
		return z_strdup(NULL, str);

	diff = (long)strlen(sub_str2) - len;

	/* allocate new memory */
	new_str = z_malloc(new_str, (size_t)(strlen(str) + count*diff + 1)*sizeof(char));

	for (q=str,t=new_str,p=str; (p = strstr(p, sub_str1)); )
	{
		/* copy until next occurrence of sub_str1 */
		for ( ; q < p; *t++ = *q++);
		q += len;
		p = q;
		for ( r = sub_str2; (*t++ = *r++); );
		--t;
	}
	/* copy the tail of str */
	for( ; *q ; *t++ = *q++ );

	*t = '\0';

	return new_str;
}

/******************************************************************************
*                                                                            *
* Function: zbx_remove_chars                                                 *
*                                                                            *
* Purpose: Remove characters 'charlist' from the whole string                *
*                                                                            *
* Parameters: str - string for processing                                    *
*             charlist - null terminated list of characters                  *
*                                                                            *
* Return value:                                                              *
*                                                                            *
* Author: Alexander Vladishev                                                *
*                                                                            *
******************************************************************************/
void	z_remove_chars(register char *str, const char *charlist)
{
	register char *p;

	if (NULL == str || NULL == charlist || '\0' == *str || '\0' == *charlist)
		return;

	for (p = str; '\0' != *p; p++)
	{
		if (NULL == strchr(charlist, *p))
			*str++ = *p;
	}

	*str = '\0';
}

char *z_nextline(char **buf) {
	char *crlf = NULL, *line = NULL;

	if (!buf || !(*buf) || !(**buf)) return NULL;

	crlf = strstr(*buf, "\r\n");
	if (!crlf) {
		line = *buf;
		*buf = NULL;
		return line;
	}
	else {
		*crlf = '\0';
		line = *buf;
		*buf = crlf += 2;
		return line;
	}
}

int z_strendswith(char *dest, const char *sep) {
	char *p = dest;
	
	if (dest == NULL || sep == NULL) return -1;

	while (*p) p++;
	p--;
	while (*sep) sep++;
	sep--;

	while (*p && *sep) {
		if (*p-- != *sep--) {
			return -1;
		}
	}
	if (*sep == '\0') return 0;
	
	return -1;
}
