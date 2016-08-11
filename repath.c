#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define __USE_GNU
#include <dlfcn.h>
 
static int (*real___lxstat)(int vers, const char *path, void *buf);
static int (*real___lxstat64)(int vers, const char *path, void *buf);
static int (*real_open)(const char *path, int flags, mode_t mode);
static int (*real_open64)(const char *path, int flags, mode_t mode);
static int (*real_creat)(const char *path, mode_t mode);
static int (*real_execve)(const char *path, char *const *argv, char *const *envp);
static int (*real_readlink)(const char *path, char *buf, size_t bufsize);
static int (*real_chmod)(const char *path, mode_t mode);

static const char *repaths = 0;
static int verbose = 0;

static void fatalf(char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	fflush(stderr);
	exit(255);
}

#define DLSYM(name) \
	do { \
		real_##name = (typeof(real_##name))dlsym(RTLD_NEXT, #name); \
		if (!real_##name) { \
			fatalf("repath.so cannot link function: %s", #name); \
		} \
	} while (0)

static void init() {
	if (repaths) {
		return;
	}
	const char *v = getenv("REPATH_VERBOSE");
	verbose  = v && v[0];
	repaths = getenv("REPATH");
	if (!repaths) {
		repaths = "";
	}

	DLSYM(__lxstat);
	DLSYM(__lxstat64);
	DLSYM(open);
	DLSYM(open64);
	DLSYM(creat);
	DLSYM(readlink);
	DLSYM(execve);
	DLSYM(chmod);
}
 
static const char *repath(const char *op, const char *path, char *buf)
{
	const char *r = repaths;
	while (*r) {
		while (*r == ',') {
			r++;
		}
		const char *colon = r;
		while (*colon != ':' && *colon != 0) {
			colon++;
		}
		const char *end = colon;
		while (*end != ',' && *end != 0) {
			end++;
		}
		const char *p = path;
		while (r < end) {
			if (*r != *p) {
				break;
			}
			r++;
			p++;
			if (*r == ':') {
				int l = end-(colon+1);
				strncpy(buf, colon+1, l);
				strncpy(buf+l, p, PATH_MAX-l);
				if (verbose) {
					fprintf(stderr, "REPATH %s %s => %s\n", op, path, buf);
				}
				return buf;
			}
		}
		r = end;
	}

	return path;
}

int __lxstat(int vers, const char *path, void *buf)
{
	init();
	char _buf[PATH_MAX];
	return real___lxstat(vers, repath("stat", path, _buf), buf);
}

int __lxstat64(int vers, const char *path, void *buf)
{
	init();
	char _buf[PATH_MAX];
	return real___lxstat64(vers, repath("stat", path, _buf), buf);
}

int open(const char *path, int flags, mode_t mode)
{
	init();
	char _buf[PATH_MAX];
	return real_open(repath("open", path, _buf), flags, mode);
}

int open64(const char *path, int flags, mode_t mode)
{
	init();
	char _buf[PATH_MAX];
	return real_open64(repath("open", path, _buf), flags, mode);
}

int creat(const char *path, mode_t mode)
{
	init();
	char _buf[PATH_MAX];
	return real_creat(repath("creat", path, _buf), mode);
}

int readlink(const char *path, char *buf, size_t bufsize)
{
	init();
	char _buf[PATH_MAX];
	return real_readlink(repath("readlink", path, _buf), buf, bufsize);
}

int execve(const char *path, char *const *argv, char *const *envp)
{
	init();
	char _buf[PATH_MAX];
	return real_execve(repath("execve", path, _buf), argv, envp);
}

int chmod(const char *path, mode_t mode)
{
	init();
	char _buf[PATH_MAX];
	return real_chmod(repath("chmod", path, _buf), mode);
}
