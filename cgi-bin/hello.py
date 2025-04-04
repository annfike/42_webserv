#!/usr/bin/python3
import cgi

form = cgi.FieldStorage()
name = form.getvalue("name")
if name:
    print(f"<h1>Hello, {name}!</h1>")
else:
    print("<h1>Hello, world!</h1>")
