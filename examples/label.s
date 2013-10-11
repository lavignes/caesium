# Infinite loop program
.entry 0 0 1
  .const 2
  .const 'test'
  add 0 256 256  # R[0] = 2 + 2
  add 0 257 257  # R[0] = 'test' + 'test'
  jmp -2
.end
