<?php
echo "Content-Type: text/html\r\n\r\n";
?>

<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>CGI PHP Test</title>
    <style>
        body {
            background-color: #f4f4f4;
            font-family: 'Segoe UI', sans-serif;
            margin: 50px;
            color: #333;
        }
        h1 {
            color: #28a745;
        }
        .env {
            margin-top: 30px;
            padding: 20px;
            background-color: #fff;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        .env h2 {
            margin-bottom: 10px;
        }
        .env pre {
            font-size: 1.2em; /* ⬅️ Taille augmentée ici */
            background-color: #f9f9f9;
            padding: 15px;
            overflow-x: auto;
            border-left: 4px solid #28a745;
            line-height: 1.6;
        }
    </style>
</head>
<body>
    <h1>✅ Test CGI PHP réussi</h1>
    <p>Ce script a été exécuté par le serveur via CGI.</p>

    <div class="env">
        <h2>Variables d'environnement CGI :</h2>
        <pre>
<?php
foreach ($_SERVER as $key => $value) {
    echo htmlspecialchars("$key = $value") . "\n";
}
?>
        </pre>
    </div>
</body>
</html>
