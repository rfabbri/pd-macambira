#ifndef _FIFO_H_
#define _FIFO_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <memory.h>
#include <pthread.h>

class Fifo
{
 public:
  Fifo();
  Fifo(unsigned int size);
  ~Fifo();
  void Flush();
  int IsAlloc(void) { return astate; }
  int ReAlloc(unsigned int size);
  void * Read(void *buf, unsigned int &len);
  int Write(void *buf, unsigned int len);
  unsigned int FreeSpace(void);
  unsigned int UsedSpace(void);
 private:
  char *buffer;
  unsigned int astate;
  unsigned int totsize, start, datasize;
  pthread_mutex_t mut;
};
#endif
