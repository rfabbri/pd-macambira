#include <stdio.h>
#include "Fifo.h"

Fifo::Fifo ()
{
  astate = 0;			// not allocated
  buffer = NULL;
  totsize = 0;
  datasize = 0;
  start = 0;
  pthread_mutex_init (&mut, 0);
}

Fifo::Fifo (unsigned int size)
{
  buffer = new char[size];
  if (buffer != NULL)
    {
      astate = 1;
      totsize = size;
      datasize = 0;
      start = 0;
    }
  else
    {
      astate = 0;
      totsize = 0;
      datasize = 0;
      start = 0;
    }
  pthread_mutex_init (&mut, 0);
}

Fifo::~Fifo ()
{
  pthread_mutex_lock (&mut);
  if (astate)
    delete buffer;
  pthread_mutex_unlock (&mut);
}

void
Fifo::Flush ()
{
  pthread_mutex_lock (&mut);
  astate = 1;
  //totsize = size;
  datasize = 0;
  start = 0;
  pthread_mutex_unlock (&mut);
  //for (int x =0; x < sizeof(buffer); x++) {
  //  buffer[x] = 0;
  //}
}

int
Fifo::ReAlloc (unsigned int size)
{
  pthread_mutex_lock (&mut);
  if (astate)
    delete buffer;
  buffer = new char[size];
  if (buffer != NULL)
    {
      astate = 1;
      totsize = size;
      datasize = 0;
      start = 0;
      pthread_mutex_unlock (&mut);
    }
  else
    {
      astate = 0;
      totsize = 0;
      datasize = 0;
      start = 0;
      pthread_mutex_unlock (&mut);
      return -1;
    }
  return 0;
}

void *
Fifo::Read (void *buf, unsigned int &len)
{
  pthread_mutex_lock (&mut);
  if (len > datasize)
    len = datasize;
  unsigned int rest;
  if (len > (totsize - start))
    rest = len - (totsize - start);
  else
    rest = 0;
  unsigned int first = len - rest;
  memcpy (buf, buffer + start, first);
  memcpy ((char *) buf + first, buffer, rest);
  datasize -= len;
  start += len;
  if (start >= totsize)
    start = rest;
  //if (datasize == 0) printf("in fifo READ, data is zero\n");
  pthread_mutex_unlock (&mut);
  return buf;
}

int
Fifo::Write (void *buf, unsigned int len)
{
  pthread_mutex_lock (&mut);
  unsigned int end;
  end = start + datasize;
  if (end > totsize)
    end = end - totsize;
  if (len > (totsize - datasize))
    {
      pthread_mutex_unlock (&mut);
      return -1;
    }
  unsigned int rest;
  if ((len + end) > totsize)
    rest = (len + end) - totsize;
  else
    rest = 0;
  unsigned int first = len - rest;
  memcpy (buffer + end, buf, first);
  memcpy (buffer, (char *) buf + first, rest);

  datasize += len;
  //if (datasize == 0) printf("in fifo WRITE, data is zero\n");
  pthread_mutex_unlock (&mut);
  return len;
}

unsigned int
Fifo::FreeSpace (void)
{				// do we need locks here?
  int x;
  pthread_mutex_lock (&mut);
  x = totsize - datasize;
  pthread_mutex_unlock (&mut);
  return x;
}

unsigned int
Fifo::UsedSpace (void)
{
  int x;
  pthread_mutex_lock (&mut);
  x = datasize;
  pthread_mutex_unlock (&mut);
  return x;
}
