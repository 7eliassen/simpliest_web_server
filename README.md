# Simple web server on C.
## Just for fun, not for prod.

processes only GET requests


Usage: %s [-a (--address) address] [-p (--port) port]
    By default 127.0.0.1:6969
    -h (--help) for display this message
    For the server to work, you need the files directory in the same directory as the program (You may use a symbolic link.)
    You also need to create a file /files/settings/redirects.settings in which to specify the
    redirection paths in the format (from:to)
    Only for GET request(yet)
