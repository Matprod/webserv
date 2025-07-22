#!/usr/bin/php-cgi
<?php
// Définir l'en-tête CGI avec \r\n pour compatibilité
header("Content-Type: text/html\r\n\r\n");

// Générer le contenu HTML
echo "<html>\n<body>\n";
echo "<h1>Hello from PHP CGI!</h1>\n";
echo "</body>\n</html>";
?>