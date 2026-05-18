<?php

// Configurações do Banco de Dados
$host = "localhost";
$dbname = "sensor";
$username = "root";
$password = ""; // Insira a senha do seu MySQL aqui, se houver

try {    
    // Conexão com o banco de dados usando PDO
    $pdo = new PDO("mysql:host=$host;dbname=$dbname;charset=utf8", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // 1. Verifica se a requisição é do tipo POST (enviada pelo ESP32 via Wi-Fi)
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {

        // Verifica se a variável 'velocidade' foi enviada
        if (isset($_POST['velocidade'])) {
            // Sanitiza e converte o valor para float
            $velocidade = filter_var($_POST['velocidade'], FILTER_VALIDATE_FLOAT);

            if ($velocidade !== false) {

                // Prepara a query SQL para evitar SQL Injection
                $sql = "INSERT INTO velocidade (velocidade) VALUES (:velocidade)";
                $stmt = $pdo->prepare($sql);                

                // Vincula o parâmetro e executa
                $stmt->bindParam(':velocidade', $velocidade);
                $stmt->execute();

                // Resposta de sucesso para o ESP32 (Ajustado para km/h conforme o cálculo do sensor)
                http_response_code(200);
                echo ">>> [DB] Salvo com sucesso: " . $velocidade . " km/h";       

            } else {
                http_response_code(400);
                echo "Erro: O valor de velocidade é inválido.";
            }

        } else {
            http_response_code(400);
            echo "Erro: Parâmetro 'velocidade' não foi enviado.";
        }

    } else {
        http_response_code(405);
        echo "Erro: Método não permitido. Utilize POST.";
    }

}

catch (PDOException $e) {
    http_response_code(500);
    echo "Erro ao inserir no MySQL: " . $e->getMessage();
}
?>