# hello world in ATOMS
# PUS
.entry 0 0 1

  .const 'Hello World!'  # K[0] = 'Hello World!'
  loadk 0 0              # R[0] = K[0]
  puts 0                 # print R[0]
  ret 0 1                # return nil

.end
