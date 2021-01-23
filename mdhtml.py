#!/usr/bin/python3

import sys
import markdown

in_filename = sys.argv[1]
out_filename = sys.argv[2]

in_fd = open(in_filename)
out_fd = open(out_filename, "w")

print(in_filename, ">>", out_filename)

out_fd.write( \
	markdown.markdown( \
		in_fd.read() \
	) \
)

