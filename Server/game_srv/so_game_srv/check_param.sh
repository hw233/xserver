#!/bin/sh
ag static --cpp  | grep -v "("  | grep -v "const" | grep -v "CMakeFiles"  | grep -v ":[0-9]*://"
