# Call a function that returns 'x'
.entry 0 0 1

  # f() { return 'x' }
  .func 0 0 1
    .const 'x'
    loadk 0 0
    ret 0 2
  .end

  # f()
  clos 0 0
  call 0 1 1
  ret 0 1
.end
