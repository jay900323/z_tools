#include "proc.h"

static const char* has_space(const char *str)
{
    const char *ch;
    for (ch = str; *ch; ++ch) {
        if (*ch == ' ') {
            return ch;
        }
    }
    return NULL;
}

int z_proc_create(z_proc_t *proc_t, const char *progname, const char * const *args, z_procattr_t *attr)
{
	int rv = 0, i = 0;
	char argv0[MAX_FILEPATH_LEN] = {0};
	char cmdline[MAX_STRING_LEN] = {0};

#ifdef _WINDOWS
    PROCESS_INFORMATION pi;
	STARTUPINFOA si;
    DWORD dwCreationFlags = 0;

	if (attr->detached) {
		dwCreationFlags |= DETACHED_PROCESS;
    }
	if (has_space(progname)) {
		char buf[MAX_FILEPATH_LEN] = {0};
		strcat(buf, "\"");
		strcat(buf, progname);
		strcat(buf, "\"");
		strcpy(argv0, buf);
    }
    else {
		strcpy(argv0, progname);
    }

    for (i = 0; args && args[i]; ++i) {
        if (has_space(args[i]) || !args[i][0]) {
			strcat(cmdline, "\"");
			strcat(cmdline, args[i]);
			strcat(cmdline, "\"");
        }
        else {
			strcat(cmdline, args[i]);
        }
    }

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if (attr->detached) {
		si.dwFlags |= STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}
	if ((attr->child_in) || (attr->child_out)
            || (attr->child_err))
    {
        si.dwFlags |= STARTF_USESTDHANDLES;

        si.hStdInput = (attr->child_in) 
                            ? attr->child_in
                            : GetStdHandle(STD_INPUT_HANDLE);

        si.hStdOutput = (attr->child_out)
                            ? attr->child_out
                            : GetStdHandle(STD_OUTPUT_HANDLE);

        si.hStdError = (attr->child_err)
                            ? attr->child_err
                            : GetStdHandle(STD_ERROR_HANDLE);
    }

    rv = CreateProcessA(progname, cmdline, /* Command line */
                        NULL, NULL,        /* Proc & thread security attributes */
                        TRUE,              /* Inherit handles */
                        dwCreationFlags,   /* Creation flags */
                        NULL,         /* Environment block */
                        attr->currdir,     /* Current directory name */
                        &si, &pi);
	if (!rv){
		//zlog_error("Error on thread exit code receiving. [%s]", strerror_from_system(GetLastError()));
        return -1;
	}

	proc_t->hproc = pi.hProcess;
    proc_t->pid = pi.dwProcessId;

	if (attr->child_in) {
        CloseHandle(attr->child_in);
	}
	if (attr->child_out) {
        CloseHandle(attr->child_out);
	}
	if (attr->child_err) {
        CloseHandle(attr->child_err);
	}
	CloseHandle(pi.hThread);

	return 0;
#else

	const char * const empty_envp[] = {NULL};

    if (attr && !attr->env) { /* Specs require an empty array instead of NULL;
                 * Purify will trigger a failure, even if many
                 * implementations don't.
                 */
        attr->env = empty_envp;
    }

	if (attr && attr->currdir) {
        if (access(attr->currdir, X_OK) == -1) {
            /* chdir() in child wouldn't have worked */
            return errno;
        }
    }
	
	if ((proc_t->pid = fork()) < 0) {
        return errno;
    }
   else if (proc_t->pid == 0) {
		/* child process */
    	if(attr){
			if ((attr->child_in == -1)) {
				close(STDIN_FILENO);
			}
			else if (attr->child_in  != STDIN_FILENO) {
				dup2(attr->child_in, STDIN_FILENO);
				close(attr->child_in);
			}

			if ((attr->child_out == -1)) {
				close(STDOUT_FILENO);

			}
			else if (attr->child_out  != STDOUT_FILENO) {
				dup2(attr->child_out, STDOUT_FILENO);
				close(attr->child_out);
			}

			if ((attr->child_err == -1)) {
				close(STDERR_FILENO);
			}
			else if (attr->child_err != STDERR_FILENO) {
				dup2(attr->child_err, STDERR_FILENO);
				close(attr->child_err);
			}

			if (attr->currdir != NULL) {
				if (chdir(attr->currdir) == -1) {
					_exit(-1);   /* We have big problems, the child should exit. */
				}
			}
    	}
    	if(attr->detached){
    		/*redirect the io to dev/null*/
    		z_proc_detach();
    	}
    	if(attr->cmdtype == Z_PROGRAM){
    		execvp(progname, (char * const *)args);
    	}
    	else if(attr->cmdtype == Z_PROGRAM_ENV){
    		execve(progname, (char * const *)args, (char * const *)attr->env);
    	}
    	else{
    		execvp(progname, (char * const *)args);
    	}
    	/*if we get here, there is a problem, so exit with a error code */
    	exit(-1);

	}
	return 0;
#endif
}

#ifndef _WINDOWS
int waitpid(pid_t pid, int *statusp, int options){
	int tmp_pid;
	/*kill 0 judge if the process exist */
	if(kill(pid, 0) == -1){
		errno = ECHILD;
		return -1;
	}
	/* wait all child process */
	while(((tmp_pid = wait3(statusp, options, 0)) != pid) &&
			(tmp_pid != -1) &&(tmp_pid != 0) &&(pid != -1));

	return tmp_pid;
}

int z_proc_detach(){
	if(freopen("/dev/null", "r", stdin) == NULL){
		return errno;
	}
	if(freopen("/dev/null", "w", stdout) == NULL){
		return errno;
	}
	if(freopen("/dev/null", "w", stderr) == NULL){
		return errno;
	}
	return 0;
}

#endif

int z_proc_wait(z_proc_t *proc, int waithow)
{
#ifdef _WINDOWS
	DWORD stat;
    DWORD time;

    if (waithow == Z_PROC_WAIT)
        time = INFINITE;
    else
        time = 0;

    if ((stat = WaitForSingleObject(proc->hproc, time)) == WAIT_OBJECT_0) {
        if (GetExitCodeProcess(proc->hproc, &stat)) {
            CloseHandle(proc->hproc);
            proc->hproc = NULL;
            return 0;
        }
    }
    else if (stat == WAIT_TIMEOUT) {
        return 0;
    }
    return -1;
#else
    pid_t pstatus;
    int waitpid_options = WUNTRACED;
    int exit_int;

    if(waithow != Z_PROC_WAIT){
    	/* WNOHANG option means return not wait until process exit */
    	waitpid_options |= WNOHANG;
    }
    do{
    	pstatus = waitpid(proc->pid, &exit_int, waitpid_options);
    }while(pstatus < 0 && errno == EINTR);

    if(pstatus > 0){
    	return 0;
    }
    else if (pstatus  == 0){
    	/* wait timeout*/
    	return -1;
    }
    return errno;
#endif
}
