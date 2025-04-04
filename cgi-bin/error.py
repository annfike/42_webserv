#!/usr/bin/python3

import traceback

try:
    print("<h1>This is an error page</h1>")
    # Deliberate ZeroDivisionError to test error handling
    print(1 / 0)  
except Exception as e:
    print("<h2>Error occurred:</h2>")
    print("<pre>")
    print(traceback.format_exc())  # Print detailed error information
    print("</pre>")
