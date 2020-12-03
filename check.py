#!/usr/bin/env python3

import json
import sys

data = json.load(sys.stdin)
reference = json.load(open(sys.argv[1]))
if data != reference:
    print("Result does not match reference.")
    print(f"reference={reference}")
    print(f"data={data}")
    sys.exit(1)
else:
    print("Correct!")
    sys.exit(0)
    
