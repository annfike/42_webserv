#!/usr/bin/python3
import time

print("Content-type: text/html\n")

current_time = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())
print(f"<h1>Current time: {current_time}</h1>")