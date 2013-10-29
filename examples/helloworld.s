# PUS params upvals stacks
.entry 0 0 1
  .const 1
  .const 2
  sub 0 256 257
  puts 0
  ret 0 1

  .resq
    catch 0
    puts 0
    ret 0 1
  .end
.end
