/**
 * cmd routing functions
*/
#pragma once

// my own header file
#include "basic.h"
#include "utils.h"
// 
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

void cmd_response(command *, state *);
void cmd_user(command *, state *);
void cmd_pass(command *, state *);
void cmd_port(command *, state *); // TODO
void cmd_pasv(command *, state *);
void cmd_cwd(command *, state *);
void cmd_pwd(command *, state *);
void cmd_list(command *, state *);
