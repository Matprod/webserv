#!/usr/bin/bash
echo "Content-Type: text/html"
echo
echo "<html><body><h2>Shell CGI Test</h2>"
echo "<p>Method: $REQUEST_METHOD</p>"
echo "</body></html>"
