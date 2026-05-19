#include "network/ClientSocket.hpp"
#include "http/RequestParser.hpp"
#include <iostream>
#include <string>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>

// Global test counters
static int g_test_count = 0;
static int g_pass_count = 0;

static void assert_test(const std::string& name, bool condition) {
    g_test_count++;
    if (condition) {
        g_pass_count++;
        std::cout << "  ✅ [PASS] " << name << std::endl;
    } else {
        std::cout << "  ❌ [FAIL] " << name << std::endl;
    }
}

// Retrieves a mock file descriptor to avoid closing standard input
static int get_mock_fd() {
    int fd = open("/dev/null", O_RDONLY);
    assert(fd >= 0 && "Failed to open /dev/null for stress testing");
    return fd;
}

// Helper to retrieve header values from HttpRequest (C++98 compliant map lookup)
static std::string get_header_val(const HttpRequest& req, const std::string& key) {
    const std::map<std::string, std::string>& headers = req.get_headers();
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

/**
 * @brief Simulador del pipeline de red asíncrono real del EventLoop.
 * Inyecta datos en el socket, los extrae vía const& y alimenta la FSM byte a byte.
 */
static e_parser_state feed_pipeline(ClientSocket& client, RequestParser& parser, const std::string& raw_packet) {
    client.append_to_read_buffer(raw_packet);
    const std::string& buffer = client.get_read_buffer();
    size_t processed = 0;
    e_parser_state state = parser.get_state();

    for (size_t i = 0; i < buffer.length(); ++i) {
        state = parser.feed(buffer[i]);
        processed++;
        if (state == STATE_ERROR || state == STATE_COMPLETE) {
            break;
        }
    }
    client.consume_read_buffer(processed);
    return state;
}

// =============================================================================
// 📑 BLOQUE 1: CASOS DE BORDE Y NORMALIZACIÓN (HTTP COMPLIANCE)
// =============================================================================

void test_header_trimming() {
    ClientSocket client(get_mock_fd());
    RequestParser parser;
    std::string req = "GET / HTTP/1.1\r\n"
                      "Host:   localhost   \r\n"
                      "X-Custom-Header:\t value_with_tabs \t\r\n\r\n";

    e_parser_state state = feed_pipeline(client, parser, req);

    bool check = (state == STATE_COMPLETE &&
                  get_header_val(parser.get_request(), "host") == "localhost" &&
                  get_header_val(parser.get_request(), "x-custom-header") == "value_with_tabs");

    assert_test("Trimming: OWS whitespace and tabs stripped", check);
}

void test_case_insensitivity() {
    ClientSocket client(get_mock_fd());
    RequestParser parser;
    std::string req = "GET / HTTP/1.1\r\n"
                      "CoNtEnT-TyPe: text/html\r\n\r\n";

    feed_pipeline(client, parser, req);

    // Comprobamos que el mapa de HttpRequest guardó la clave en minúsculas estrictas
    bool lower_saved = parser.get_request().get_headers().count("content-type") == 1;
    bool caps_rejected = parser.get_request().get_headers().count("CoNtEnT-TyPe") == 0;

    assert_test("Normalization: Header key converted to lowercase", lower_saved && caps_rejected);
}

// =============================================================================
// ⚡ BLOQUE 2: COMPORTAMIENTO ASÍNCRONO Y FRAGMENTACIÓN EXTREMA
// =============================================================================

void test_torture_fragmentation() {
    ClientSocket client(get_mock_fd());
    RequestParser parser;
    std::string full_request = "POST /submit HTTP/1.1\r\n"
                               "Content-Length: 5\r\n"
                               "Connection: close\r\n\r\n"
                               "12345";

    e_parser_state state = parser.get_state();
    // Simulamos que la tarjeta de red entrega los datos de UN carácter en UN carácter (peor escenario)
    for (size_t i = 0; i < full_request.length(); ++i) {
        std::string single_char_packet(1, full_request[i]);
        state = feed_pipeline(client, parser, single_char_packet);
    }

    assert_test("Network: 1-byte extreme stream fragmentation survival",
        state == STATE_COMPLETE &&
        parser.get_request().get_body() == "12345");
}

// =============================================================================
// 🛡️ BLOQUE 3: MITIGACIÓN DE ATAQUES Y SEGURIDAD (ANTIVULNERABILIDADES)
// =============================================================================

void test_invalid_header_key_spaces() {
    ClientSocket client(get_mock_fd());
    RequestParser parser;
    // RFC 7230: No se permiten espacios dentro del nombre de una cabecera (intento de split/smuggling)
    std::string req = "GET / HTTP/1.1\r\n"
                      "Bad Header: value\r\n\r\n";

    e_parser_state state = feed_pipeline(client, parser, req);
    assert_test("Security: Rejection of space inside Header Key", state == STATE_ERROR);
}

void test_malformed_content_length() {
    ClientSocket client(get_mock_fd());
    RequestParser parser;
    // Envío de basura en el Content-Length para reventar el stringstream
    std::string req = "POST / HTTP/1.1\r\n"
                      "Content-Length: 42abc\r\n\r\n";

    e_parser_state state = feed_pipeline(client, parser, req);
    assert_test("Security: Rejection of toxic alphanumeric Content-Length", state == STATE_ERROR);
}

void test_negative_content_length() {
    ClientSocket client(get_mock_fd());
    RequestParser parser;
    // Intento de desbordamiento por enteros negativos
    std::string req = "POST / HTTP/1.1\r\n"
                      "Content-Length: -50\r\n\r\n";

    e_parser_state state = feed_pipeline(client, parser, req);
    assert_test("Security: Rejection of negative Content-Length layout", state == STATE_ERROR);
}

void test_version_overflow_attack() {
    ClientSocket client(get_mock_fd());
    RequestParser parser;
    // Intento de inundar la memoria enviando una versión gigante sin saltar de línea
    std::string req = "GET / HTTP/1.1111111111111111111111111111111111111111111";

    e_parser_state state = feed_pipeline(client, parser, req);
    assert_test("Security: Early detection of protocol version buffer overflow", state == STATE_ERROR);
}

// =============================================================================
// 🧠 PANEL CENTRAL DE EJECUCIÓN
// =============================================================================

int main() {
    std::cout << "===============================================================\n";
    std::cout << "🧪 WEBSERV PARSER STRESS & ROBUSTNESS TEST SUITE (C++98 COMPLIANT)\n";
    std::cout << "===============================================================\n";

    std::cout << "[➔] Block 1: Compliance & Edge Cases Normalization\n";
    test_header_trimming();
    test_case_insensitivity();

    std::cout << "\n[➔] Block 2: Asynchronous Network Simulation\n";
    test_torture_fragmentation();

    std::cout << "\n[➔] Block 3: Security & Attack Vector Mitigation\n";
    test_invalid_header_key_spaces();
    test_malformed_content_length();
    test_negative_content_length();
    test_version_overflow_attack();

    std::cout << "===============================================================\n";
    std::cout << "📊 SUMMARY: " << g_pass_count << " / " << g_test_count 
              << " tests successfully passed.\n";
    std::cout << "===============================================================\n";

    return (g_pass_count == g_test_count) ? 0 : 1;
}
