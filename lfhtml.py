#!/usr/bin/python3

import sys

filename = sys.argv[1]
outname = sys.argv[2]
i_fd = open(filename, "r")
o_fd = open(outname, "w")


print(filename, ">>", outname)

o_fd.write(i_fd.read().replace('\n', "<br>"))

i_fd.close()
o_fd.close()


