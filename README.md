# Caesium 

[![Build Status](https://secure.travis-ci.org/pyrated/caesium.png)](http://travis-ci.org/pyrated/caesium)

Caesium is a register-based virtual machine for a luaVM-like byte-code called ATOMS.
By default, Caesium comes with an high-level ATOMS assembler supporting multiprocessing, automatic memory management, first class functions, classes, and inter-thread
channel communication.

usage:
  
`$ bin/caesium examples/helloworld.s`

## Features

* Built-In Assembler

* Simple and fast register-based (No Stack) architecture

* Designed with multi-threading in mind

* High Level: Automatic memory management, polymorphic class system, exception handling, first class functions

* RISC: Less than 40 opcodes

* Immutable Data-Types: including UTF-8 strings

* Reentrant C API for writing ludicrously fast modules