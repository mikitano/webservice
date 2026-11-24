// ============================================================================
// test_server.cpp - main de teste do Server (listeners + LoopingEvent + Client).
// O config é MONTADO NA MÃO: cada argumento vira um server block com essa porta.
//
// COMO COMPILAR E RODAR:
//   make test_server
//   ./test_server 4242 4243          (sem argumentos: porta 8080)
//   make test_server && valgrind --leak-check=full --track-fds=yes ./test_server 4242
//
// Em OUTRO terminal, rode os testes da etapa em que você está.
// Todos devem terminar com o Ctrl+C no servidor imprimindo o encerramento limpo.
//
// ----------------------------------------------------------------------------
// ETAPA 1 - listeners + accept (o Server só aceita e fecha o fd) OK
// ----------------------------------------------------------------------------
//   ss -ltnp | grep -E "4242|4243" -> duas linhas LISTEN em 0.0.0.0, com o nome do programa
//   nc -zv 127.0.0.1 4242        	-> "succeeded!"; o servidor loga o accept
//   nc -zv 127.0.0.1 4243        	-> idem, na outra porta
//   ./test_server 4242 4242      	-> erro de bind na 2a e saída com código 1,
//                                  		sem vazar o fd da 1a (confira com valgrind)
//   ./test_server 80             	-> "Permission denied" (sem sudo)
//											sem vazar o fd da 1a (confira com valgrind)
//
// ----------------------------------------------------------------------------
// ETAPA 2 - Client criado no accept + leitura (POLLIN) - EM PROGRESSO
// ----------------------------------------------------------------------------
//   nc 127.0.0.1 4242            -> digite texto: o servidor loga "recv N bytes"
//   (feche o nc com Ctrl+D)      -> o servidor loga o cliente fechado (recv 0)
//   (printf "GET / HT"; sleep 1; printf "TP/1.1\r\n\r\n") | nc 127.0.0.1 4242
//                                -> PARSE_INCOMPLETE e depois PARSE_DONE
//   for i in 1 2 3; do nc 127.0.0.1 4242 & done
//                                -> 3 clientes ao mesmo tempo, fds diferentes
//   ls /proc/$(pgrep -x test_server)/fd
//                                -> depois de fechar os nc, só 0,1,2 e os listeners
//
// ----------------------------------------------------------------------------
// ETAPA 3 - escrita (POLLOUT)
// ----------------------------------------------------------------------------
//   curl -v 127.0.0.1:4242/abc               -> 200 "Voce pediu: GET /abc"
//   curl -s -d "abcde" 127.0.0.1:4243/x      -> 200, corpo recebido pelo parser
//   curl -s -i -X FOO 127.0.0.1:4242/ | head -1
//                                            -> HTTP/1.1 400 Bad Request
//   curl -s 127.0.0.1:4242/big | wc -c       -> 5242880
//   curl -s --limit-rate 2000k 127.0.0.1:4242/big -o /dev/null
//                                            -> VÁRIOS envios parciais no log
//   strace -e trace=poll -p $(pgrep -x test_server)    (instale: sudo apt install strace)
//        -> ocioso: só POLLIN; durante o /big lento: POLLIN|POLLOUT;
//           terminou: volta a só POLLIN (POLLOUT desligado)
//   (Ctrl+C no curl do /big lento)           -> servidor fecha o cliente e segue de pé
//
// ----------------------------------------------------------------------------
// ETAPA 4 - keep-alive e timeout
// ----------------------------------------------------------------------------
//   curl -v 127.0.0.1:4242/a 127.0.0.1:4242/b
//        -> "Re-using existing connection": 2 respostas na mesma conexão
//   curl --http1.0 -v 127.0.0.1:4242/   -> o servidor fecha a conexão depois da resposta
//   nc 127.0.0.1 4242   (e não digite nada)
//        -> depois do CLIENT_TIMEOUT_SEC o servidor fecha. Para testar rápido,
//           troque a constante por 5 e recompile.
//   nc ocioso + em outro terminal: curl 127.0.0.1:4242/
//        -> o curl responde na hora (o nc ocioso não trava os outros)
//   for i in $(seq 20); do nc -w1 127.0.0.1 4242 < /dev/null; done
//   ls /proc/$(pgrep -x test_server)/fd | wc -l
//        -> mesmo número de fds de antes (sem vazamento)
//
// ----------------------------------------------------------------------------
// ETAPA 5 - estresse e limpeza (antes de integrar o CGI)
// ----------------------------------------------------------------------------
//   siege -c 100 -r 10 http://127.0.0.1:4242/   -> Availability: 100.00 %
//   valgrind --leak-check=full --track-fds=yes ./test_server 4242
//        (aperte Ctrl+C depois dos testes)
//        -> "All heap blocks were freed" e só os fds 0,1,2 abertos no fim
// ============================================================================

#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include "config/model/Config.hpp"
#include "network/Server.hpp"

// PROVISÓRIO: no lugar do parser de config
static Config buildMockConfig(int argc, char** argv) {
	Config config;

	for (int i = 1; i < argc; ++i) {
		ServerConfig server;
		server.setPort(std::atoi(argv[i]));
		server.addServerName("localhost");
		config.addServer(server);
	}
	if (config.getServers().empty()) {			// sem argumentos: porta padrão
		ServerConfig server;
		server.setPort(8080);
		config.addServer(server);
	}
	return config;
}

int main(int argc, char** argv) 
{
	Config config = buildMockConfig(argc, argv);

	try 
	{
		Server server(config.getServers());
		server.run();
	} catch (const std::exception& e) 
	{
		std::cerr << "erro: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
