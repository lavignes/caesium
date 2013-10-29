# PUS params upvals stacks
.entry 0 0 1
  .const 0
  .const 2
  loadk 0 0
  add 0 0 257
  puts 0
  jmp -2
  ret 0 1
  .resq
    catch 0
    puts 0
    ret 0 1
  .end
.end
