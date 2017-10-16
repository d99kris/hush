/*
 * faketty.c
 *
 * Copyright (C) 2017 Kristofer Berggren
 * All rights reserved.
 * 
 * faketty is distributed under the BSD 3-Clause license, see LICENSE for details.
 *
 */

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 600
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>


#define max(a, b)  ({ __typeof__ (a) _a = (a);  __typeof__ (b) _b = (b); _a > _b ? _a : _b; })


static void showusage(void);
static void showversion(void);


static void showusage(void)
{
  printf("Faketty executes specified program with its stdin, stdout and stderr\n"
         "routed through pseudo-terminal (PTY) devices. This can be used to\n"
         "trick shell programs that they are being run in an interactive shell.\n"
         "It may also be used for automation purposes or for simply redirecting\n"
         "color output to files.\n"
         "\n"
         "Usage: faketty PROG [ARGS..]\n"
         "\n"
         "Examples:\n"
         "faketty cc 2> /tmp/err.txt ; cat /tmp/err.txt\n"
         "        calls system C compiler without any input file specified, redirecting\n"
         "        its error message to a file, and then viewing the file content.\n"
         "\n"
         "Report bugs at https://github.com/d99kris/hush\n"
         "\n");
}

static void showversion(void)
{
  printf("faketty v1.0\n"
         "\n"
         "Copyright (C) 2017 Kristofer Berggren\n"
         "\n"
         "faketty is distributed under the BSD 3-Clause license.\n"
         "\n"
         "Written by Kristofer Berggren\n"
         "\n");
}


int main(int argc, char **argv)
{
  int rv = 0;
  char *ptyname = NULL;

  /* check arguments */
  if (argc == 1)
  {
    showusage();
    return 1;
  }
  else if ((argc == 2) && (strcmp(argv[1], "--help") == 0))
  {
    showusage();
    return 0;
  }
  else if ((argc == 2) && (strcmp(argv[1], "--version") == 0))
  {
    showversion();
    return 0;
  }

  /* set up pty for stdin and stdout */
  int parent_pty_stdio = posix_openpt(O_RDWR | O_NOCTTY);
  if ((parent_pty_stdio == -1) || (grantpt(parent_pty_stdio) == -1) ||
      (unlockpt(parent_pty_stdio) == -1) || ((ptyname = ptsname(parent_pty_stdio)) == NULL))
  {
    fprintf(stderr, "faketty: error opening parent stdio pty, exiting.\n");
    return -1;
  }

  int child_pty_stdio = open(ptyname, O_RDWR | O_NOCTTY);
  if (child_pty_stdio < 0)
  {
    fprintf(stderr, "faketty: error opening child stdio pty, exiting.\n");
    return -1;
  }

  /* set up pty for stderr */
  int parent_pty_stderr = posix_openpt(O_RDONLY | O_NOCTTY);
  if ((parent_pty_stderr == -1) || (grantpt(parent_pty_stderr) == -1) ||
      (unlockpt(parent_pty_stderr) == -1) || ((ptyname = ptsname(parent_pty_stderr)) == NULL))
  {
    fprintf(stderr, "faketty: error opening parent stderr pty, exiting.\n");
    return -1;
  }

  int child_pty_stderr = open(ptyname, O_WRONLY | O_NOCTTY);
  if (child_pty_stderr < 0)
  {
    fprintf(stderr, "faketty: error opening child stderr pty, exiting.\n");
    return -1;
  }
  
  /* fork */
  pid_t pid = fork();
  if (pid > 0)
  {
    /* parent */
    close(child_pty_stdio);
    close(child_pty_stderr);

    /* main parent loop */
    while (1)
    {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(STDIN_FILENO, &fds);
      FD_SET(parent_pty_stdio, &fds);
      FD_SET(parent_pty_stderr, &fds);

      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10000;

      int sel = select(max(parent_pty_stdio, parent_pty_stderr) + 1, &fds, NULL, NULL, &tv);
      if (sel > 0)
      {
        /* pass stdin data to child process */
        if (FD_ISSET(STDIN_FILENO, &fds))
        {
          char buf[BUFSIZ];
          int len = read(STDIN_FILENO, buf, sizeof(buf));
          if (len > 0)
          {
            write(parent_pty_stdio, buf, len);
          }
        }

        /* pass child process out to stdout */
        if (FD_ISSET(parent_pty_stdio, &fds))
        { 
          char buf[BUFSIZ];
          int len = read(parent_pty_stdio, buf, sizeof(buf));
          if (len > 0)
          {
            write(STDOUT_FILENO, buf, len);
          }
        }

        /* pass child process err to stderr */
        if (FD_ISSET(parent_pty_stderr, &fds))
        { 
          char buf[BUFSIZ];
          int len = read(parent_pty_stderr, buf, sizeof(buf));
          if (len > 0)
          {
            write(STDERR_FILENO, buf, len);
          }
        }
      }

      /* check if child process is still running */
      int status = 0;
      if (waitpid(pid, &status, WNOHANG) != 0)
      {
        rv = WEXITSTATUS(status);
        break;
      }
    }

    /* parent clean up */
    close(parent_pty_stdio);
    close(parent_pty_stderr);
  }
  else
  {
    /* child */
    close(parent_pty_stdio);
    close(parent_pty_stderr);

    /* configure termio */
    struct termios ctermios;
    if (tcgetattr(child_pty_stdio, &ctermios) == 0)
    {
      cfmakeraw(&ctermios);
      tcsetattr(child_pty_stdio, TCSANOW, &ctermios);
    }

    if (tcgetattr(child_pty_stderr, &ctermios) == 0)
    {
      cfmakeraw(&ctermios);
      tcsetattr(child_pty_stderr, TCSANOW, &ctermios);
    }

    /* close original stdin, stdout, stderr */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* replace stdin, stdout and stderr with ptys */
    dup2(child_pty_stdio, STDIN_FILENO);
    dup2(child_pty_stdio, STDOUT_FILENO);
    dup2(child_pty_stderr, STDERR_FILENO);

    /* close original ptys */
    close(child_pty_stdio);
    close(child_pty_stderr);

    /* stdin tty config */
    setsid();
    ioctl(STDIN_FILENO, TIOCSCTTY, 1);

    /* execute child program */
    execvp(argv[1], argv + 1); 
  }
  
  return rv;
}
  
