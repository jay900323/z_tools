#ifndef _DIRENT_H
#define _DIRENT_H
#ifdef __cplusplus
extern "C" {
#endif
	
#ifdef _WIN32

#include <sys/types.h>

struct dirent
{
	long d_ino;              /* inode number*/
	off_t d_off;             /* offset to this dirent*/
	unsigned short d_reclen; /* length of this d_name*/
	unsigned char d_type;    /* the type of d_name*/
	char d_name[256];          /* file name (null-terminated)*/
};

typedef struct _dirdesc {
	int     dd_fd;      /** file descriptor associated with directory */
	long    dd_loc;     /** offset in current buffer */
	long    dd_size;    /** amount of data returned by getdirentries */
	char    *dd_buf;    /** data buffer */
	int     dd_len;     /** size of data buffer */
	long    dd_seek;    /** magic cookie returned by getdirentries */
	int finished;
	HANDLE handle;
	struct dirent entry;
} DIR;

# define __dirfd(dp)    ((dp)->dd_fd)

DIR *z_opendir(const char *name);
struct dirent *z_readdir(DIR *dp);
int z_closedir(DIR *d);
int z_readdir_r(DIR *dp, struct dirent* entry, struct dirent **result);

/*∑«µ›πÈ…®√Ë*/
int z_scandir(const char *dirname, struct dirent **namelist[], int(*selector) (const struct dirent *entry), int(*compare) (const struct dirent **a, const struct dirent **b));

#elif __linux__

#endif

int z_get_exec_path(char *dir, int len);
int z_chdir(const char *dir);
	
#ifdef __cplusplus
}
#endif
#endif
