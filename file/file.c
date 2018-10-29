#include "file.h"
#include "common.h"
#include <stdio.h>

#ifdef _WINDOWS
#include <Windows.h>
#elif __linux__
#include <unistd.h>
#endif

int z_is_file_exist(const char *file)
{
#ifdef _WINDOWS
	DWORD dwAttrib = GetFileAttributes(file);
	if (INVALID_FILE_ATTRIBUTES != dwAttrib /*&& 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)*/) {
		return 0;
	}
	else {
		return -1;
	}
#elif __linux__
	return access(file, F_OK);
#endif
}

int z_file_read(const char *filename, char **buf, int *len)
{
	FILE *fp = 0;
	int bytesread = 0;
	int ret = 1;
	char *buffer = NULL;


	if ((fp = fopen(filename, "rb")) == NULL) {
		return -1;
	}
	*len = 0;

	fseek(fp, 0L, SEEK_END);
	bytesread = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	buffer = *buf = (char*)malloc(bytesread+1);
	memset(buffer, 0, bytesread+1);
	while (ret) {
		ret = fread(buffer, 1, 1024, fp);
		if (ret <= 0) {
			fclose(fp);
			return 0;
		}
		buffer += ret;
		*len += ret;
	}
	fclose(fp);

	return 0;
}

int z_file_write(const char *filename, const char *buf, int len) {
	int ret = 1;
	FILE *fd;

	if ((fd = fopen(filename, "wb")) == NULL) {
		return -1;
	}
	while (len) {
		ret = fwrite(buf, 1, len, fd);
		if (ret <= 0) {
			fclose(fd);
			return 0;
		}
		len -= ret;
		buf += ret;
	}
	fclose(fd);
	return 0;
}

int z_file_size(const char *filename)
{
	FILE *fp = 0;
	int bytesread = 0;

	if ((fp = fopen(filename, "rb")) == NULL) {
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	bytesread = ftell(fp);
	fclose(fp);

	return bytesread;
}

int z_delete_file(const char *path)
{
#ifdef _WIN32
	if (DeleteFile(path)) {
		return 0;
	}
	return -1;
#elif __linux__
	if (unlink(path) == 0) {
		return 0;
	}
	return -1;
#endif
}

char* z_full_path(const char *relative_path, char *fullpath, int full_len)
{
#ifdef _WIN32
	return _fullpath(fullpath, relative_path, full_len);
#else
	return realpath(relative_path, fullpath);
#endif
}

int z_file_rename(const char *oldname, const char *newname)
{
#ifdef _WIN32
	if (GetFileAttributes(oldname) != -1) {
		MoveFileEx(oldname, newname, MOVEFILE_REPLACE_EXISTING);
	}
#elif __linux__
	rename(oldname, newname);
#endif
}