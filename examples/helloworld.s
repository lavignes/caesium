# PUS params upvals stacks
.entry 0 0 1
  .const 'CoolError'
  
  loadk 0 0
  raise 0

  ret 0 1

  .resq
    .const 'Rescue from error:'
    loadk 0 1
    puts 0
    catch 0
    puts 0
  .end
.end
