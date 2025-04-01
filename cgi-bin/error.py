#!/usr/bin/python3

print("Content-type: text/html\n")
print("<h1>This is an error page</h1>")
print(1 / 0)  # Deliberate ZeroDivisionError to test error handling
