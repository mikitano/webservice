// ============================================================================
// test_client.cpp - testa SÓ o Client, usando um socketpair (sem rede, sem poll).
//
// Um socketpair cria DUAS pontas ligadas entre si:
//   fds[0] -> entregue ao Client (é a "conexão aceita")
//   fds[1] -> fica com o teste, fingindo ser o cliente remoto (nc/curl)
//
// COMO RODAR:
//   make test_client
//   ./test_client
//   valgrind --leak-check=full --track-fds=yes ./test_client     (opcional)
//
// ============================================================================

#include <sys/socket.h>
#include "network/Client.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "utils/Utils.hpp"


int	main()
{
	
	/* --- socket de escuta (so o necessario pra testar o Client) --- */
	int	listenFd = socket(AF_INET, SOCK_STREAM, 0);
	int	yes = 1;
	setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	struct sockaddr_in	addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(8086);
	if (bind(listenFd, (struct sockaddr *)&addr, sizeof(addr)) < 0
		|| listen(listenFd, 10) < 0)
	{
		std::cerr << "bind/listen falhou" << std::endl;
		return (1);
	}
	std::cout << "esperando um cliente na porta 8086..." << std::endl;

	/* --- accept: o kernel preenche clientAddr --- */
	struct sockaddr_in	clientAddr;
	socklen_t			clientLen = sizeof(clientAddr);
	int					clientFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLen);
	if (clientFd < 0)
	{
		std::cerr << "accept falhou" << std::endl;
		close(listenFd);
		return (1);
	}
	std::cout << "accept na port 8086 funcionou" << std::endl;
	

	// /* --- cria o Client ja com tudo: fd, IP e porta --- */
	Client	*client = new Client(clientFd, clientAddr);

	std::cout << "Client criado: fd = " << client->getFdClient()
		<< ", remoteAddr = " << client->getInfo().getRemoteAddr()
		<< ", remotePort = " << client->getInfo().getRemotePort() << std::endl;

	delete client;	/* o destrutor fecha o fd: o cliente deve ver a conexao fechar */
	std::cout << "Client destruido (fd fechado)" << std::endl;

	close(listenFd);
	return (0);
}
