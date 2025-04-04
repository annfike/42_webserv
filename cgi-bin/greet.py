#!/usr/bin/python3
import cgi # type: ignore

print("Content-type: text/html\n")

form = cgi.FieldStorage()
greeting = form.getvalue("greeting")
if greeting:
    print(f"<h1>Your greeting: {greeting}</h1>")
else:
    print("<h1>No greeting provided.</h1>")
