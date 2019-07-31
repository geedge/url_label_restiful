#!/bin/sh
valgrind --tool=memcheck --leak-check=full --leak-resolution=high --error-limit=no --undef-value-errors=yes --show-reachable=yes --log-file=valgrind.log --malloc-fill=AA --free-fill=FE --max-stackframe=7418264 ./web_focus
