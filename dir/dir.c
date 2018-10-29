#include <stdio.h>
#include "common.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>  
#include "dir.h"

DIR *z_opendir(const char *name)
{
	DIR *dir;
	WIN32_FIND_DATA FindData = {0};
	char dirname[512];
	HANDLE handle;

	dir = (DIR *)malloc(sizeof(DIR));
	if (!dir) {
		return 0;
	}
	memset(dir, 0, sizeof(DIR));
	strcpy(dirname, name);
	strcat(dirname, "\\*.*");

	//if (handle = FindFirstFileEx(dirname, FindExInfoBasic, &FindData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH) == INVALID_HANDLE_VALUE) {
	if ((handle = FindFirstFile(dirname, &FindData)) == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err == ERROR_NO_MORE_FILES || err == ERROR_FILE_NOT_FOUND) {
			dir->finished = 1;
		}
		else {
			free(dir);
			return NULL;
		}
	}

	dir->dd_fd = 0;
	dir->handle = handle;
	return dir;
}

struct dirent *z_readdir(DIR *dp)
{
	int i;
	BOOL bf;
	WIN32_FIND_DATA FileData;

	if (!dp || dp->finished)
		return NULL;

	bf = FindNextFile(dp->handle, &FileData);
	if (!bf)
	{
		dp->finished = 1;
		return 0;
	}

	for (i = 0; i < 256; i++)
	{
		dp->entry.d_name[i] = FileData.cFileName[i];
		if (FileData.cFileName[i] == '\0') break;
	}
	dp->entry.d_reclen = i;
	dp->entry.d_reclen = FileData.nFileSizeLow;

	if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		dp->entry.d_type = 2; /*目录*/
	}
	else
	{
		dp->entry.d_type = 1; /*文件*/
	}

	return (&dp->entry);
}

int z_readdir_r(DIR *dp, struct dirent* entry, struct dirent **result)
{
	int i ;
	BOOL bf ;
	WIN32_FIND_DATA FileData ;

	if(!dp || dp->finished) {
		*result = NULL ;
		return 0 ;
	}
	
	bf = FindNextFile(dp->handle, &FileData) ;
	if(!bf) {
		dp->finished = 1 ;
		*result = NULL ;
		return 0 ;
	}

	for(i = 0 ; i < 256 ; i++) {
		dp->entry.d_name[i] = FileData.cFileName[i] ;
		if(FileData.cFileName[i] == '\0') break ;
	}
	dp->entry.d_reclen = i ;
	dp->entry.d_reclen = FileData.nFileSizeLow ;

	if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		dp->entry.d_type = 2 ;  /*目录*/
	}
	else {
		dp->entry.d_type = 1 ; /*文件*/
	}
	memcpy(entry, &dp->entry, sizeof(*entry)) ;

	*result = &dp->entry ;

	return 0 ;
}

int z_closedir(DIR *d)
{
	if(!d) return - 1 ;
	FindClose(d->handle) ;
	free(d) ;
	return 0 ;
}

int z_scandir(const char *dirname, struct dirent **namelist[], int(*selector)(const struct dirent *entry), int(*compare)(const struct dirent **a, const struct dirent **b))
{
	DIR *dirp = NULL ;
	struct dirent **vector = NULL ;
	int vector_size = 0 ;
	int nfiles = 0 ;
	char entry[sizeof(struct dirent)] ;
	struct dirent *dp = (struct dirent *)&entry ;

	if(namelist == NULL) {
	return - 1 ;
	}

	if(!(dirp = z_opendir(dirname))) {
		return - 1 ;
	}

	while(!z_readdir_r(dirp, (struct dirent *)entry, &dp) && dp) {
		int dsize = 0 ;
		struct dirent *newdp = NULL ;

		if(selector && (*selector)(dp) == 0) {
		continue ;
	}

	if(nfiles == vector_size) {
		struct dirent **newv ;
		if(vector_size == 0) {
		vector_size = 10 ;
	}
	else {
		vector_size *= 2 ;
	}

	newv = (struct dirent **) realloc(vector, vector_size * sizeof(struct dirent *)) ;
	if(!newv) {
		return - 1 ;
	}
	vector = newv ;
	}

	dsize = sizeof(struct dirent) ;
	newdp = (struct dirent *) malloc(dsize) ;

	if(newdp == NULL) {
		goto fail ;
	}

	vector[nfiles++] = (struct dirent *) memcpy(newdp, dp, dsize) ;
	}

	z_closedir(dirp) ;

	*namelist = vector ;

	if(compare) {
		qsort(*namelist, nfiles, sizeof(struct dirent *), (int(*)(const void *, const void *)) compare) ;
	}

	return nfiles ;

	fail :
		while(nfiles-- > 0) {
		free(vector[nfiles]) ;
	}
	free(vector) ;
	return - 1 ;
}

#elif __linux__


#endif

int z_get_exec_path(char *dir, int len)
{
	int i = 0;
	
#ifdef _WIN32
	GetModuleFileName(NULL, dir, len);
#else
	readlink("/proc/self/exe", dir, len);
#endif
	for (i = (int)strlen(dir); i > 0; i--)
	{
		if (dir[i] == DEFAULT_SLASH)
		{
			dir[i + 1] = '\0';
			break;
		}
	}
	return 0;
}


int z_chdir(const char *dir) 
{
#ifdef _WIN32
	SetCurrentDirectory(dir);
#elif __linux__
	chdir(dir);
#endif
}