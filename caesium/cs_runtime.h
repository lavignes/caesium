#ifndef _CS_RUNTIME_H_
#define _CS_RUNTIME_H_

#include <pthread.h>
#include <semaphore.h>

#include "cs_list.h"
#include "cs_hash.h"

typedef struct CsRuntime {

  pthread_mutex_t globals_lock; // Lock for globals table
  pthread_mutex_t gc_lock;      // Lock to control the gc state
  sem_t gc_sync;                // Used to synchronize gc
  pthread_cond_t gc_done;       // Used to relieve gc lock contention
  CsHash* globals;
  CsList* mutators;
  int nprocs;

} CsRuntime;

CsRuntime* cs_runtime_new();

void cs_runtime_free(CsRuntime* cs);

void cs_runtime_dofile(CsRuntime* cs, const char* filename);

/**
 * Assemble and run Caesium ATOM bytecode
 * @param cs    Caesium runtime handle
 * @param u8str utf-8 encoded assembly file contents
 * @param size  size of string in bytes
 */
void cs_runtime_doassembly(CsRuntime* cs, const char* u8str, size_t size);

void cs_runtime_dostring(CsRuntime* cs, const char* u8str);

#endif /* _CS_RUNTIME_H_ */