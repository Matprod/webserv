#!/usr/bin/php-cgi
<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<html><body>";
echo "<h2>PHP CGI Test</h2>";
echo "<p>Method: " . $_SERVER["REQUEST_METHOD"] . "</p>";
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    foreach ($_POST as $key => $value) {
        echo "<p>$key = $value</p>";
    }
}
echo "</body></html>";
?>
