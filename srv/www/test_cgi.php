<?php
header("Content-Type: text/html");

echo "<html><head><title>Test CGI PHP</title></head><body>";
echo "<h1>Test CGI PHP - Succès!</h1>";
echo "<h2>Variables d'environnement CGI:</h2>";
echo "<ul>";

// Afficher les variables d'environnement importantes
$env_vars = [
    'REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_LENGTH', 'CONTENT_TYPE',
    'SCRIPT_NAME', 'SCRIPT_FILENAME', 'PATH_INFO', 'SERVER_NAME',
    'SERVER_PORT', 'SERVER_PROTOCOL', 'GATEWAY_INTERFACE'
];

foreach ($env_vars as $var) {
    $value = $_SERVER[$var] ?? 'Non défini';
    echo "<li><strong>$var:</strong> $value</li>";
}

echo "</ul>";

// Afficher les données POST si présentes
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $content_length = (int)($_SERVER['CONTENT_LENGTH'] ?? 0);
    if ($content_length > 0) {
        $post_data = file_get_contents('php://input');
        echo "<h2>Données POST reçues:</h2>";
        echo "<pre>" . htmlspecialchars($post_data) . "</pre>";
    }
}

echo "</body></html>";
?>
