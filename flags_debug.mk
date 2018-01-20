# Initial compiler flags and linker flags 
# to build a non optimized program which can be debugged.

CF := -std=gnu89 -Wall -Wextra -Werror -pedantic-errors -g \
      -O0 -DDBG \
      -DFILE_BUF_SIZE=5U
LF := -std=gnu89 -lrt -Wl,-v

