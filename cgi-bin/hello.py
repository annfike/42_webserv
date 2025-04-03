#!/usr/bin/python3
import cgi
import os
import sys

print(os.environ["QUERY_STRING"], file=sys.stderr)

print("Content-type: text/html\n")
form = cgi.FieldStorage()
name = form.getvalue("name")
if name:
    print(f"<h1>Hello, {name}!</h1>")
else:
    print("<h1>Hello, world!</h1>")
