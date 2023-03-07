#!/bin/bash
tmux splitw -h -l 120 "gdbserver :12345 ./demo"
tmux selectp -t 0
gdb -x debug.gdb

