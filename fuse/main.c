/*
 * https://raw.githubusercontent.com/libfuse/libfuse/46b9c3326d50aebe52c33d63885b83a47a2e74ea/example/hello.c
 */

/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos file_sizeeredi <miklos@file_sizeeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char src[PATH_MAX];

static struct options {
	const char *srcdir;
} options;

#define OPTION(t, p) \
	{ t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
	OPTION("--srcdir=%s", srcdir),
	FUSE_OPT_END
};

char* concat(const char *path1, const char *path2)
{
    char *res = malloc(strlen(path1) + strlen(path2) + 1);
    strcpy(res, path1);
    strcat(res, path2);
    return res;
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
	stat(concat(src, path), stbuf);
	return 0;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;
	
	DIR *srcdir = opendir(concat(src, path));
	if (srcdir == NULL) 
		return -ENOENT;

	struct dirent *de;
	while (de = readdir(srcdir)) {
		struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
	}

	closedir (srcdir);

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{	
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	FILE *fp = fopen(concat(src, path), "r");
	fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
	size_t len = 0;
    fseek(fp, 0, SEEK_SET);

    char *data = malloc(file_size);
	char c;
    while ((c = fgetc(fp)) != EOF) {
		if ('0' <= c && c <= '9')
			c = (char) ('a' + c - '0');
		else if ('a' <= c && c <= 'z')
			c = (char) ('A' + c - 'a');
        data[len++] = (char) c;
    }
    data[len] = '\0';

	if (offset < file_size) {
		if (offset + size > file_size)
			size = file_size - offset;
		memcpy(buf, data + offset, size);
	} 
	else
		size = 0;

	fclose(fp);

	return size;
}

static struct fuse_operations hello_oper = {
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
};

int main(int argc, char *argv[])
{
    int option_index = 0;
	static struct option long_options[] = {
		{"srcdir", required_argument, 0, 's'}
	};
	int opt = getopt_long(argc, argv, "srcdir", long_options, &option_index);
    realpath(optarg, src);  

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
		return 1;
	}

	return fuse_main(args.argc, args.argv, &hello_oper, NULL);
}
