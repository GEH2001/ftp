#pragma once
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Using popen() to Obtain a List of Files from the ls Utility */
int list_files(char *buf, int buf_size, const char *path);

/**
 * Using popen() to Obtain a List of Files from the ls Utility.
 * Write through the data socket.
*/
int write_list_files(int sock_data, const char *path);