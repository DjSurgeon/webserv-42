// Copyright 2026 serjimen vja-nie dlesieur
#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <string>

#include "http/RequestParser.hpp"
#include "network/ClientSocket.hpp"

// Helper para imprimir el resultado visual de cada test
static void print_test_result(const std::string& test_name, bool success) {
  if (success) {
    std::cout << "✅ [PASS] " << test_name << std::endl;
  } else {
    std::cout << "❌ [FAIL] " << test_name << std::endl;
  }
}

// Retorna un descriptor de archivo único e independiente para cada test
static int get_mock_fd() {
  int fd = open("/dev/null", O_RDONLY);
  assert(fd >= 0 && "Failed to open /dev/null for integration testing");
  return fd;
}

/**
 * @brief Simula el comportamiento del EventLoop real conectando el buffer y el
 * parser. Consume los caracteres del ClientSocket uno a uno y limpia el buffer
 * al terminar.
 */
static e_parser_state run_integration_pipeline(
    ClientSocket* client, RequestParser* parser,
    const std::string& raw_network_data) {
  if (!client || !parser) {
    return STATE_ERROR;
  }
  // 1. El cliente recibe datos de la red y los inyecta en su búfer
  client->appendToReadBuffer(raw_network_data);

  // 2. Leemos la referencia constante del búfer de lectura
  const std::string& buffer = client->getReadBuffer();
  size_t bytes_processed = 0;
  e_parser_state current_state = parser->get_state();

  // 3. Alimentamos la FSM carácter a carácter
  for (size_t i = 0; i < buffer.length(); ++i) {
    current_state = parser->feed(buffer[i]);
    bytes_processed++;

    if (current_state == STATE_ERROR || current_state == STATE_HEADER_KEY) {
      // Detener el procesamiento si ocurre un error o si se completa la
      // Request-Line
      break;
    }
  }

  // 4. El parser le indica al cliente cuántos bytes del búfer puede consumir
  // (borrar)
  client->consumeReadBuffer(bytes_processed);
  return current_state;
}

// --- BATERÍA DE CASOS DE PRUEBA ---

void test_standard_request() {
  ClientSocket client(get_mock_fd());
  RequestParser parser;

  std::string data = "GET /index.html HTTP/1.1\r\n";
  e_parser_state final_state = run_integration_pipeline(&client, &parser, data);

  bool check =
      (final_state == STATE_HEADER_KEY) &&
      (parser.get_request().get_method() == "GET") &&
      (parser.get_request().get_uri() == "/index.html") &&
      (parser.get_request().get_version() == "HTTP/1.1") &&
      (client.getReadBuffer().empty());  // Todo el buffer debió ser consumido

  print_test_result("Standard Valid Request Line", check);
}

void test_fragmented_network_stream() {
  ClientSocket client(get_mock_fd());
  RequestParser parser;

  // Simula que la red entrega la petición en 3 ráfagas de bytes separadas
  run_integration_pipeline(&client, &parser, "GE");
  run_integration_pipeline(&client, &parser, "T /search?q=42 HT");
  e_parser_state final_state =
      run_integration_pipeline(&client, &parser, "TP/1.1\r\n");

  bool check = (final_state == STATE_HEADER_KEY) &&
               (parser.get_request().get_method() == "GET") &&
               (parser.get_request().get_uri() == "/search?q=42") &&
               (client.getReadBuffer().empty());

  print_test_result("Fragmented TCP Network Stream", check);
}

void test_tolerant_leading_garbage() {
  ClientSocket client(get_mock_fd());
  RequestParser parser;

  // RFC 7230 3.5: Tolerar líneas vacías flotantes previas
  std::string data = "\r\n\r\nPOST /upload HTTP/1.1\r\n";
  e_parser_state final_state = run_integration_pipeline(&client, &parser, data);

  bool check = (final_state == STATE_HEADER_KEY) &&
               (parser.get_request().get_method() == "POST") &&
               (parser.get_request().get_uri() == "/upload");

  print_test_result("RFC 7230 Tolerant Leading Garbage", check);
}

void test_attack_double_space() {
  ClientSocket client(get_mock_fd());
  RequestParser parser;

  // Error de protocolo: Doble espacio entre método y URI
  std::string data = "GET  /malformed HTTP/1.1\r\n";
  e_parser_state final_state = run_integration_pipeline(&client, &parser, data);

  print_test_result("Attack Mitigation: Double Space",
                    final_state == STATE_ERROR);
}

void test_attack_wrong_version() {
  ClientSocket client(get_mock_fd());
  RequestParser parser;

  // Solo debemos aceptar HTTP/1.1 de forma estricta en esta fase
  std::string data = "GET / HTTP/1.2\r\n";
  e_parser_state final_state = run_integration_pipeline(&client, &parser, data);

  print_test_result("Attack Mitigation: HTTP/1.2 Rejection",
                    final_state == STATE_ERROR);
}

int main() {
  std::cout << "🧪 RUNNING WEBSERV INTEGRATION TEST BATTERY (CLIENT_SOCKET + "
               "PARSER)\n";
  std::cout
      << "---------------------------------------------------------------\n";

  test_standard_request();
  test_fragmented_network_stream();
  test_tolerant_leading_garbage();
  test_attack_double_space();
  test_attack_wrong_version();

  std::cout
      << "---------------------------------------------------------------\n";
  return 0;
}
