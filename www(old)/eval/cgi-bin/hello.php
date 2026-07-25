#!/usr/bin/env php-cgi
<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<html><head><title>PHP CGI</title></head><body>";
echo "<h1>Hello from PHP CGI! 🐘</h1>";
echo "<h2>Environment Variables:</h2>";
echo "<ul>";
foreach ($_SERVER as $key => $value) {
    echo "<li><strong>$key</strong>: $value</li>";
}
echo "</ul>";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $body = file_get_contents("php://input");
    echo "<h2>POST Body Received:</h2>";
    echo "<pre>$body</pre>";
}
echo "</body></html>";
?>
