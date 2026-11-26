import os
import sys

length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
data = sys.stdin.read(length)

print("Content-Type: text/plain")
print()
print("Metodo: " + os.environ.get("REQUEST_METHOD", ""))
print("Recebi " + str(len(data)) + " bytes: " + data)
