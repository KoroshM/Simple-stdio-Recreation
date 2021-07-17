/** @file stdio.cpp
 * @author Korosh Moosavi
 * @date 2021-05-25
 *
 * Modified version of stdio_template.cpp provided by Prof. Dimpsey
 *
 * stdio.cpp file:
 * The .cpp file contains all of the method implementations except
 *   for the constructor included in the .h file
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
 */

#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
using namespace std;

char decimal[100];

/////////////////////////////////////////////////
// Untouched methods provided by Prof. Dimpsey //
/////////////////////////////////////////////////

int recursive_itoa(int arg)
{
   int div = arg / 10;
   int mod = arg % 10;
   int index = 0;
   if (div > 0)
   {
      index = recursive_itoa(div);
   }
   decimal[index] = mod + '0';
   return ++index;
}

char* itoa(const int arg)
{
   bzero(decimal, 100);
   int order = recursive_itoa(arg);
   char* new_decimal = new char[order + 1];
   bcopy(decimal, new_decimal, order + 1);
   return new_decimal;
}

int printf(const void* format, ...)
{
   va_list list;
   va_start(list, format);

   char* msg = (char*)format;
   char buf[1024];
   int nWritten = 0;

   int i = 0, j = 0, k = 0;
   while (msg[i] != '\0')
   {
      if (msg[i] == '%' && msg[i + 1] == 'd')
      {
         buf[j] = '\0';
         nWritten += write(1, buf, j);
         j = 0;
         i += 2;

         int int_val = va_arg(list, int);
         char* dec = itoa(abs(int_val));
         if (int_val < 0)
         {
            nWritten += write(1, "-", 1);
         }
         nWritten += write(1, dec, strlen(dec));
         delete dec;
      }
      else
      {
         buf[j++] = msg[i++];
      }
   }
   if (j > 0)
   {
      nWritten += write(1, buf, j);
   }
   va_end(list);
   return nWritten;
}

int setvbuf(FILE* stream, char* buf, int mode, size_t size)
{
   if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
   {
      return -1;
   }
   stream->mode = mode;
   stream->pos = 0;
   if (stream->buffer != (char*)0 && stream->bufown == true)
   {
      delete stream->buffer;
   }

   switch (mode)
   {
   case _IONBF:
      stream->buffer = (char*)0;
      stream->size = 0;
      stream->bufown = false;
      break;
   case _IOLBF:
   case _IOFBF:
      if (buf != (char*)0)
      {
         stream->buffer = buf;
         stream->size = size;
         stream->bufown = false;
      }
      else
      {
         stream->buffer = new char[BUFSIZ];
         stream->size = BUFSIZ;
         stream->bufown = true;
      }
      break;
   }
   return 0;
}

void setbuf(FILE* stream, char* buf)
{
   setvbuf(stream, buf, (buf != (char*)0) ? _IOFBF : _IONBF, BUFSIZ);
}

FILE* fopen(const char* path, const char* mode)
{
   FILE* stream = new FILE();
   setvbuf(stream, (char*)0, _IOFBF, BUFSIZ);

   // fopen( ) mode
   // r or rb = O_RDONLY
   // w or wb = O_WRONLY | O_CREAT | O_TRUNC
   // a or ab = O_WRONLY | O_CREAT | O_APPEND
   // r+ or rb+ or r+b = O_RDWR
   // w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
   // a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

   switch (mode[0])
   {
   case 'r':
      if (mode[1] == '\0')  // r
      {
         stream->flag = O_RDONLY;
      }
      else if (mode[1] == 'b')
      {
         if (mode[2] == '\0')  // rb
         {
            stream->flag = O_RDONLY;
         }
         else if (mode[2] == '+')  // rb+
         {
            stream->flag = O_RDWR;
         }
      }
      else if (mode[1] == '+')  // r+  r+b
      {
         stream->flag = O_RDWR;
      }
      break;

   case 'w':
      if (mode[1] == '\0')  // w
      {
         stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
      }
      else if (mode[1] == 'b')
      {
         if (mode[2] == '\0')  // wb
         {
            stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
         }
         else if (mode[2] == '+')  // wb+
         {
            stream->flag = O_RDWR | O_CREAT | O_TRUNC;
         }
      }
      else if (mode[1] == '+')  // w+  w+b
      {
         stream->flag = O_RDWR | O_CREAT | O_TRUNC;
      }
      break;

   case 'a':
      if (mode[1] == '\0')  // a
      {
         stream->flag = O_WRONLY | O_CREAT | O_APPEND;
      }
      else if (mode[1] == 'b')
      {
         if (mode[2] == '\0')  // ab
         {
            stream->flag = O_WRONLY | O_CREAT | O_APPEND;
         }
         else if (mode[2] == '+')  // ab+
         {
            stream->flag = O_RDWR | O_CREAT | O_APPEND;
         }
      }
      else if (mode[1] == '+')  // a+  a+b
      {
         stream->flag = O_RDWR | O_CREAT | O_APPEND;
      }
      break;
   }

   mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

   if ((stream->fd = open(path, stream->flag, open_mode)) == -1)
   {
      delete stream;
      printf("fopen failed\n");
      stream = NULL;
   }

   return stream;
}

///////////////////////////////////////
// Methods written by Korosh Moosavi //
///////////////////////////////////////

// -------------------------------------------------------------- fpurge(FILE*)
// This method wipes the data in the file buffer by replacing every element
//   with '\0'
// The file buffer's position and actual size are reset to 0
// 
// param: stream  Pointer to the file object whose buffer is being cleared
// 
// pre:    The file has been initialized an opened
// post:   File buffer is refilled with \0, actual size & position are reset
// return: 0 on success, -1 on error
//
int fpurge(FILE* stream)
{
   if (stream == nullptr)                    // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (stream->mode == _IONBF)               // No-buffer check
   {
      return -1;
   }

   stream->fpos -= stream->actual_size - stream->pos;
   lseek(stream->fd, 0, stream->fpos);       // Backtrack if buffer is unused

   for (int i = 0; i < stream->size; i++)    // Loop through every element
   {
      stream->buffer[i] = '\0';              // Replace with '\0'
   }

   stream->pos = 0;                          // Reset position & actual size
   stream->actual_size = 0;
   stream->lastop = 0;
   return 0;
} // end fpurge

// -------------------------------------------------------------- fflush(FILE*)
// This method prints what remains in the buffer to the file
// Buffer is then purged
// 
// param: stream  Pointer to the file object whose buffer is being flushed
// 
// pre:    The file has been initialized an opened
// post:   Remaining file buffer is written and purged
// return: 0 on success, -1 on error
//
int fflush(FILE* stream)
{
   if (stream == nullptr)                    // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (stream->lastop == 'w')                // Set actual size to correct val
   {
      stream->actual_size = stream->pos;
   }
   if (stream->mode == _IONBF)               // No-buffer check
   {
      return -1;
   }

   write(stream->fd, stream->buffer, stream->actual_size);
   stream->fpos += stream->actual_size;

   fpurge(stream);
   return 0;
} // end fflush

// -------------------------------------------------------------- refill(FILE*)
// Reads from the file and fills the buffer
// Sets file's EOF field to true if amount read is less than full buffer
// Used by read methods when buffer is empty or fully used
// 
// param: stream  Pointer to the file object whose buffer is being refilled
// 
// pre:    The file has been initialized an opened
// post:   File buffer is refilled if data remains in the file
//         EOF is set to true if amount read is less than buffer size
//
void refill(FILE* stream)
{
   if (stream == nullptr)                    // Parameter validation
   {
      printf("Null file parameter");
      return;
   }
   if (stream->mode == _IONBF)               // No-buffer check
   {
      return;
   }

   stream->pos = 0;
   stream->actual_size = read(stream->fd, stream->buffer, stream->size);
   if (stream->actual_size == -1)
   {
      printf("Error in reading file\n");     // read() returns -1 on error
      return;
   }
   if (stream->actual_size < stream->size)
   {
      stream->eof = true;                    // Set EOF if needed
   }
   return;
} // end refill

// ---------------------------------------- fread(void*, size_t, size_t, FILE*)
// Outputs a given amount of memory from the file buffer to the user buffer
// Reads from the file directly if memory requested is multiple times 
//   the max file buffer size
// 
// param: ptr     Pointer to an index in the user buffer
// param: size    Byte size of one unit in the user buffer
// param: nmemb   Number of units to read
// param: stream  Pointer to the file object being read from
// 
// pre:    The file has been initialized an opened
// post:   Requested amount of memory is read from the file to the user
//           buffer, up to the end of the file
//         EOF is set to true if amount read is less than buffer size
//
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
   if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) ||
      stream->flag == (O_WRONLY | O_CREAT | O_APPEND))
   {
      printf("Read permissions not granted\n"); // Permissions check
      return -1;
   }
   if (stream == nullptr)                       // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (size < 1 || nmemb < 1)                   // Parameter validation
   {
      printf("nmemb must be > 0\n");
      return -1;
   }
   if ((stream->pos == stream->actual_size &&
      (stream->actual_size > 0 || stream->pos > 0))
      || stream->lastop == 0)                   // Buffer empty or used up
   {
      refill(stream);
      if (stream->actual_size == -1)
      {
         return -1;
      }
   }
   if (stream->lastop == 'w')                   // Flush written data
   {
      fflush(stream);
   }

   size_t totalMem = size * nmemb;              // Total memory needed
   size_t offset = 0;                           // Total memory read
   char* buf = (char*)ptr;                      // Current user buffer position
   char* sbuf = stream->buffer + stream->pos;   // Current file buffer position

   // EOF reached in buffer
   if (stream->eof)                             // EOF read into buffer
   {
      if (stream->pos != stream->actual_size)   // Buffer not finished
      {
         offset = stream->actual_size - stream->pos;
         memcpy(buf, sbuf, offset);             // Finish reading buffer

         stream->pos = stream->actual_size;
         stream->lastop = 'r';
         return offset;
      }
      else                                      // Buffer finished
      {
         stream->lastop = 'r';
         fpurge(stream);                        // Just clear buffer & return 0
         return 0;
      }
   }

   // No buffer allowed
   if (stream->size == 0 || stream->mode == _IONBF) // No buffer
   {
      stream->actual_size = read(stream->fd, buf, totalMem); // Read from disk

      if (stream->actual_size != -1)
      {
         if (stream->actual_size < stream->size)
         {
            stream->eof = true;
         }
         stream->fpos += stream->actual_size;
         stream->lastop = 'r';
         return stream->actual_size;
      }
   }

   // Requested memory is less than remaining unread buffer contents
   if (totalMem + stream->pos <= stream->size)  // Wants unread mem in buffer
   {
      if (stream->actual_size != -1)
      {
         memcpy(buf, sbuf, totalMem);           // Read the mem into user buff
         buf += totalMem;                       // Move user buffer pointer
         sbuf += totalMem;                      // Move file buffer pointer
         offset += totalMem;                    // Track amount of mem passed
         stream->pos += totalMem;               // Track position in file buff
         stream->fpos += totalMem;              // Track position in file
      }
   }
   // Requested memory is more than remaining unread buffer contents
   else                                         // Wants mem past end of buffer
   {
      memcpy(buf, sbuf, stream->size - stream->pos);  // Read remaining buffer
      buf += stream->size - stream->pos;
      offset += stream->size - stream->pos;
      stream->pos = 0;

      while (totalMem - offset >= stream->size && !stream->eof)
      {                                   // Loop until remainig mem < max buff
         stream->actual_size = read(stream->fd, buf, stream->size);
         if (stream->actual_size == -1)
         {
            break;
         }
         if (stream->actual_size < stream->size)
         {
            stream->eof = true;
         }
         buf += stream->actual_size;
         offset += stream->actual_size;
         stream->fpos += stream->actual_size;
      }

      if (offset <= totalMem && !stream->eof)   // Read remaining mem needed
      {
         refill(stream);
         if (stream->actual_size != -1)
         {
            sbuf = stream->buffer;

            memcpy(buf, sbuf, totalMem - offset);
            stream->pos += totalMem - offset;
            stream->fpos += totalMem - offset;
            buf += totalMem - offset;
            sbuf += totalMem - offset;
            offset = totalMem;
         }
      }
      else
      {
         stream->pos = stream->actual_size;
      }
   }

   stream->lastop = 'r';
   if (stream->actual_size == -1)               // read() returns -1 on error
      return -1;
   else
      return offset;
} // end fread

// --------------------------------- fwrite(const void*, size_t, size_t, FILE*)
// Inputs data from the user buffer into the stream buffer then into the file
// 
// param: ptr     Pointer to an index in the user buffer
// param: size    Byte size of one unit in the user buffer
// param: nmemb   Number of units to read
// param: stream  Pointer to the file object being written to
// 
// pre:    The file has been initialized an opened
// post:   Requested amount of memory is read from the file to the user
//           buffer, up to the end of the file
//         EOF is set to true if amount read is less than buffer size
//
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
   if (stream->flag == O_RDONLY)                      // Permissions check
   {
      printf("Write permissions not granted\n");
      return -1;
   }
   if (stream->flag == (O_WRONLY | O_CREAT | O_APPEND) ||
      stream->flag == (O_RDWR | O_CREAT | O_APPEND))  // Append check
   {
      stream->fpos = lseek(stream->fd, 0, SEEK_END);
   }
   if (stream == nullptr)                             // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (size < 1 || nmemb < 1)                         // Parameter validation
   {
      printf("Invalid memory parameter");
      return -1;
   }


   size_t totalMem = size * nmemb;                    // Total mem to write
   size_t written = 0;                                // Total written mem
   size_t offset = 0;                                 // Written per loop
   char* in = (char*)ptr;                             // Pointer to user buffer

   if (stream->size == 0 || stream->mode == _IONBF)   // No buffer
   {
      written = write(stream->fd, stream->buffer, totalMem);
      stream->fpos += written;

      stream->lastop = 'w';
      return written;
   }

   if (stream->lastop == 'r')                         // Purge after reads
   {
      fpurge(stream);
   }

   while (written < totalMem) // Loop until requested amount of mem is read
   {
      offset = 0;

      while (*in != '\0' && offset < stream->size && offset < totalMem)
      {                       // Loop until end of input, buffer, or total mem
         stream->buffer[offset] = *in;
         in++;
         offset++;
      }

      write(stream->fd, stream->buffer, offset);      // Write to file
      stream->fpos += offset;                         // Advance file position

      written += offset;
   }

   stream->lastop = 'w';
   return written;
} // end fwrite

// --------------------------------------------------------------- fgetc(FILE*)
// Read a single character from the file/buffer and return it
// 
// param: stream  Pointer to the file object being read from
// 
// pre:    The file has been initialized an opened
// post:   One char will have been read from the file or buffer
// return: char value that was read
//
int fgetc(FILE* stream)
{
   if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) ||
      stream->flag == (O_WRONLY | O_CREAT | O_APPEND))// Permissions check
   {
      printf("Read permissions not granted\n");
      return -1;
   }
   if (stream == nullptr)                             // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (stream->size == 0 || stream->mode == _IONBF)   // No buffer
   {
      char c;
      stream->actual_size = read(stream->fd, &c, 1);  // Read from disk
      stream->lastop = 'r';

      if (stream->actual_size != -1)
      {
         if (stream->actual_size == 0)
         {
            stream->eof = true;
         }
         stream->fpos += stream->actual_size;         // Advance file position
         return (int)c;
      }

      else
      {
         return -1;
      }
   }
   if (stream->lastop == 'w')                         // Flush written data
   {
      fflush(stream);
   }
   if (stream->eof && stream->pos == stream->actual_size)// Out of data
   {
      fpurge(stream);
      return -1;
   }
   if (stream->pos == stream->actual_size ||
      stream->lastop == 0)                            // Buffer empty/used up
   {
      refill(stream);
   }

   char c;                                            // char to return
   char* sbuf = stream->buffer + stream->pos;         // Current buff position

   memcpy(&c, sbuf, 1);                               // Get char
   stream->pos++;                                     // Advance buff pos
   stream->fpos++;                                    // Advance file pos
   stream->lastop = 'r';

   if (stream->eof && stream->pos == stream->actual_size)
   {
      fpurge(stream);                                 // Clear used up buffer
   }

   return (int)c;
} // end fgetc

// ---------------------------------------------------------- fputc(int, FILE*)
// Writes a single char into the file
// 
// param: stream  Pointer to the file object being written to
// param: inputChar  char to write to the file
// 
// pre:    The file has been initialized an opened
// post:   inputChar will exist in the file data
// return: char that was input (inputChar parameter)
//
int fputc(int inputChar, FILE* stream)
{
   if (stream->flag == O_RDONLY)                // Permissions check
   {
      printf("Write permissions not granted\n");
      return -1;
   }
   if (stream->flag == (O_WRONLY | O_CREAT | O_APPEND) ||
      stream->flag == (O_RDWR | O_CREAT | O_APPEND))// Append check
   {
      stream->fpos = lseek(stream->fd, 0, SEEK_END);
   }
   if (stream == nullptr)                       // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (inputChar < 0)                           // Parameter validation
   {
      printf("Invalid char parameter");
      return -1;
   }
   if (stream->mode == _IONBF)                  // No-buffer check
   {
      write(stream->fd, &inputChar, 1);
      return inputChar;
   }
   if (stream->lastop == 'r')                   // Purge if last op was read
   {
      fpurge(stream);
   }
   if (stream->pos == stream->size)             // Flush if buffer is filled
   {
      fflush(stream);
   }

   char* sbuf = stream->buffer + stream->pos;   // Point to current position in buffer
   memcpy(sbuf, &inputChar, 1);
   stream->pos++;
   stream->fpos++;

   stream->lastop = 'w';
   if (stream->pos == stream->size)
   {
      fflush(stream);                           // Flush if buffer is filled
   }

   return inputChar;
} // end fputc

// --------------------------------------------------- fgets(char*, int, FILE*)
// Read a string from the file/buffer
// Up to parameter-dictated size of bytes
// Returns a series of bytes ending in '\0'
// 
// param: str     User's string buffer
// param: size    Max size of user buffer
// param: stream  Pointer to the file object being read from
// 
// pre:    File has been opened and initialized
// post:   The next string of bytes in the file is read
// return: The string of bytes read from the file
//
char* fgets(char* str, int size, FILE* stream)
{
   if (stream->flag == (O_WRONLY | O_CREAT | O_TRUNC) ||
      stream->flag == (O_WRONLY | O_CREAT | O_APPEND))
   {                                // Permissions check
      printf("Read permissions not granted\n");
      return NULL;
   }
   if (stream == nullptr)           // Parameter validation
   {
      printf("Null file parameter");
      return NULL;
   }
   if (size < 0)                    // Parameter validation
   {
      printf("Invalid size parameter");
      return NULL;
   }
   if (stream->eof && stream->pos == stream->actual_size)
   {                                // Out of data
      return NULL;
   }
   if (stream->lastop == 'w')       // Flush written data
   {
      fflush(stream);
   }

   char c;                          // Last char read
   char* strcur = str;              // Current user buff position
   int i = 0;                       // Size (bytes) read

   while ((c = fgetc(stream)) != EOF && c != '\n')
   {                 // Loop until EOF, new line, or user buff size is reached
      if (i < (size - 2))
      {
         *strcur = c;
         strcur++;
         i++;
      }
      else                          // User buffer size - 1 reached
      {
         *strcur = c;
         strcur++;
         break;
      }
   }
   stream->lastop = 'r';

   if (i == size - 2)               // Add final char to fill user buffer
   {
      *strcur = fgetc(stream);
      strcur++;
   }
   else if (*--strcur != '\n')      // Append \n if needed
   {                                // Otherwise write() overwrites its outputs
      *++strcur = '\n';
      *++strcur = '\0';
   }

   return str;
} // end fgets

// -------------------------------------------------- fputs(const char*, FILE*)
// Writes a string into the file
// 
// param: str     User string to be written
// param: stream  Pointer to the file object being written to
// 
// pre:    File has been opened and initialized
// post:   Data in user buffer has been written to the file
// return: Number of bytes written
//
int fputs(const char* str, FILE* stream)
{
   if (stream->flag == O_RDONLY)                      // Permissions check
   {
      printf("Write permissions not granted\n");
      return -1;
   }
   if (stream == nullptr || str == nullptr)           // Parameter validation
   {
      printf("Null pointer parameter");
      return -1;
   }
   if (stream->flag == (O_WRONLY | O_CREAT | O_APPEND) ||
      stream->flag == (O_RDWR | O_CREAT | O_APPEND))  // Append check
   {
      stream->fpos = lseek(stream->fd, 0, SEEK_END);
   }
   if (stream->mode == _IONBF)                        // No-buffer check
   {
      return write(stream->fd, str, strlen(str));
   }

   char c;                                            // char in user string
   int i = 0;                                         // chars written

   while ((c = fputc(str[i], stream)) != '\0' && c != '\n')
   {
      i++;                 // Write 1 char at a time until end of user string
   }

   return i;
} // end fputs

// ---------------------------------------------------------------- feof(FILE*)
// Returns whether EOF has been reached for the file
// 
// param: stream  Pointer to the file object being checked
// 
// pre:    File has been opened and initialized
// return: Whether the EOF field has been set to true in the file object
//
int feof(FILE* stream)
{
   if (stream == nullptr)                             // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }

   return stream->eof == true;
} // end feof

// ---------------------------------------------------- fseek(FILE*, long, int)
// Moves the current position within the file as dictated by the parameters
// Negative offset results in moving backwards
// Final position is offset + whence in the file
// Sets EOF field to its correct value
// Allows repositioning past the end of the file
// 
// param: stream  Pointer to the file object being repositioned
// 
// pre:    File has been opened and initialized
// post:   File position is at indicated location
// return: 0 on success, -1 on error
//
int fseek(FILE* stream, long offset, int whence)
{
   if (stream == nullptr)                          // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (whence < 0)                                 // Parameter validation
   {
      printf("Starting point must be >= 0");
      return -1;
   }
   if (stream->lastop == 'w')                      // Flush written data
   {
      fflush(stream);
   }

   lseek(stream->fd, 0, 0);
   int fileSize = lseek(stream->fd, 0, SEEK_END);  // Get max file size


   if (offset < 0)                                 // Relocate backwards
   {
      if (whence == SEEK_END)                      // Relocate from EOF
      {
         stream->fpos = fileSize + offset;         // Calc final position
         lseek(stream->fd, stream->fpos, 0);       // Move to it
      }

      else                                         // Relocate from file pos
      {
         stream->fpos = stream->fpos + offset;
         lseek(stream->fd, stream->fpos, 0);
      }
   }

   else                                            // Relocate forwards
   {
      stream->fpos = whence + offset;
      lseek(stream->fd, stream->fpos, 0);
   }

   if (stream->fpos >= fileSize)                   // Set EOF when applicable
      stream->eof = true;
   else
      stream->eof = false;

   return 0;
} // end fseek

// -------------------------------------------------------------- fclose(FILE*)
// Deletes file buffer if owned by the parameter object
// Closes the file descriptor for the file
// 
// param: stream  Pointer to the file object being closed
// 
// pre:    File has been opened and initialized
// post:   File is closed
// return: Return value of close()
//
int fclose(FILE* stream)
{
   if (stream == nullptr)                          // Parameter validation
   {
      printf("Null file parameter");
      return -1;
   }
   if (stream->lastop == 'w')                      // Flush written data
   {
      fflush(stream);
   }
   else if (stream->lastop == 'r')                 // Purge buffer
   {
      fpurge(stream);
   }
   if (stream->bufown)                             // Delete buffer if owned
   {
      delete stream->buffer;
   }

   delete stream;                                  // Open new / Close delete
   return close(stream->fd);                       // Close file, exeunt
} // end fclose
