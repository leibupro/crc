# Initial compiler flags and linker flags 
# to build an optimized program.

CF := -std=gnu89 -Wall -Wextra -Werror -pedantic-errors -s\
      -O3 -fforce-addr -funroll-loops -frerun-cse-after-loop \
      -frerun-loop-opt -falign-functions=8 -flto

LF := -std=gnu89 -flto -O3 -lrt -Wl,-v

