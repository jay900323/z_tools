#ifndef _ZFILE_H
#define _ZFILE_H

#ifdef __cplusplus
extern "C" {
#endif

int z_is_file_exist(const char *file);
int z_file_read(const char *filename, char **buf, int *len);
int z_file_size(const char *filename);
int z_file_write(const char *filename, const char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif