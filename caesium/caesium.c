#include "cs_runtime.h"

int main(int argc, char** argv) {

  CsRuntime* cs = cs_runtime_new();

  if (argc > 1) {
    cs_runtime_dofile(cs, argv[1]);
  }

  cs_runtime_free(cs);
  return 0;
}