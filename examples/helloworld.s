# print X -- X isn’t a defined global variable
# Output will be:
#
.entry 0 0 1
  .const 'X'
  loadg 0 0    # Exception will occur here, NameError
  puts 0
  ret 0 1
  .resq
    catch 0
    puts 0
    ret 0 1
  .end
.end
