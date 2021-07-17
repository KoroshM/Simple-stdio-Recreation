/** @file stdio.h
 * @author Korosh Moosavi
 * @date 2021-05-25
 *
 * Modified version of stdio.h provided by Prof. Dimpsey
 *
 * stdio.h file:
 * The .h file contains all of the data fields and implementation
 *   of the constructor
 * The only real change made to the .h was the inclusion of a 
 *   file position tracker 'fpos' which tracks the current position
 *   in the file as an offset (in bytes) from the start
 * This file is a highly simplified version of <stdio.h> included
 *   in the standard library
 * This class is able to open, close, read, and write to files
 *
 * Assumptions:
 * Append mode requires user to reposition after each write
 *   if switching to read (write will move to EOF)
 * The underlying system calls work as intended
 * The user is responsible for using fseek() to reposition
 *   after writing in append mode, if attempting to read
 * A user calling fpurge() does so knowing all the content
 *   in the buffer will be overwritten with \0
 * fseek() sets EOF when applicable
 * The actual_size member is only updated on read() calls
 *   as a reference for the number of bytes last read
 *   Used to check for if EOF was reached inside the buffer
 */

#ifndef _MY_STDIO_H_
#define _MY_STDIO_H_

#define BUFSIZ 8192 // default buffer size
#define _IONBF 0    // unbuffered
#define _IOLBF 1    // line buffered
#define _IOFBF 2    // fully buffered
#define EOF -1      // end of file

class FILE 
{
 public:
  FILE() 
  {
     fd = 0;
     pos = 0;
     fpos = 0;
     buffer = (char *) 0;
     size = 0;
     actual_size = 0;
     mode = _IONBF;
     flag = 0;
     bufown = false;
     lastop = 0;
     eof = false;
  }


  int fd;          // a Unix file descriptor of an opened file
  int pos;         // the current file position in the buffer
  int fpos;        // the current file position in the file
  char *buffer;    // an input or output file stream buffer
  int size;        // the buffer size
  int actual_size; // the actual buffer size when read( ) returns # bytes read smaller than size
  int mode;        // _IONBF, _IOLBF, _IOFBF
  int flag;        // O_RDONLY 
                   // O_RDWR 
                   // O_WRONLY | O_CREAT | O_TRUNC
                   // O_WRONLY | O_CREAT | O_APPEND
                   // O_RDWR   | O_CREAT | O_TRUNC
                   // O_RDWR   | O_CREAT | O_APPEND
  bool bufown;     // true if allocated by stdio.h or false by a user
  char lastop;     // 'r' or 'w' 
  bool eof;        // true if EOF is reached
};
#include "stdio.cpp"
#endif
