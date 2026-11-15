// ============================================================================
// test_loopingevent.cpp - testa SÓ o LoopingEvent, usando um pipe (sem sockets).
//
// COMO RODAR:
//   make test_loop
//   ./test_loop
//
// RESULTADO ESPERADO: todas as linhas com [ OK ] e no fim "0 falharam".
// Se alguma der [FALHOU], o nome do teste diz qual comportamento está errado.
//
// O QUE CADA TESTE PROVA:
//   1. wait() sem nada pronto devolve 0 (e respeita o timeout).
//   2. Dado chegando no pipe aparece como "readable".
//   3. setPollOut liga/desliga o POLLOUT (writable só aparece quando ligado).
//   4. setPollIn liga/desliga o POLLIN.
//   5. removeFd tira o fd do vetor E apaga o evento pronto desse fd.
//   6. addFdPoll do mesmo fd duas vezes não duplica.
//   7. Fechar a ponta de escrita aparece como "error" (POLLHUP) na ponta de leitura.
//   8. removeFd de um fd que não existe não quebra nada.
//
// DICA DE DEPURAÇÃO: se o programa travar, o wait() está bloqueando: confira se
// o timeout está indo para o poll() e se o vetor de pollfd não está vazio.
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include "network/LoopingEvent.hpp"
#include "utils/Logger.hpp"

static int	g_total = 0;
static int	g_failed = 0;

static void check(bool ok, const std::string& name) {
	++g_total;
	if (!ok)
		++g_failed;
	std::cout << (ok ? "[ OK ] " : "[FALHOU] ") << name << std::endl;
}

static const LoopingEvent::FdEvent* findEvent(const std::vector<LoopingEvent::FdEvent>& events, int fd) {
	for (size_t i = 0; i < events.size(); ++i)
		if (events[i]._fd == fd)
			return &events[i];
	return NULL;
}

static void drainOne(int fd) {
	char c;
	read(fd, &c, 1);
}

int main() {
	int p[2];
	if (pipe(p) < 0) {
		std::cerr << "pipe falhou" << std::endl;
		return 1;
	}
	int readFd = p[0];
	int writeFd = p[1];
	fcntl(readFd, F_SETFL, O_NONBLOCK);
	fcntl(writeFd, F_SETFL, O_NONBLOCK);

	LoopingEvent loop;

	// ---- 1. nada pronto ----
	loop.addFdPoll(readFd);
	check(loop.hasFd(readFd), "1. addFdPoll: hasFd devolve true");
	check(loop.size() == 1, "1. addFdPoll: size == 1");
	check(loop.wait(100) == 0, "1. sem dados: wait(100) devolve 0");
	check(loop.getReady().empty(), "1. sem dados: getReady vazio");

	// ---- 2. dado chegando ----
	write(writeFd, "x", 1);
	check(loop.wait(100) == 1, "2. com dado: wait devolve 1");
	const LoopingEvent::FdEvent* ev = findEvent(loop.getReady(), readFd);
	check(ev != NULL && ev->_readable, "2. com dado: evento readable no readFd");
	check(ev != NULL && !ev->_writable && !ev->_error, "2. com dado: sem writable nem error");
	drainOne(readFd);

	// ---- 3. setPollOut ----
	loop.addFdPoll(writeFd);
	check(loop.size() == 2, "3. segundo fd: size == 2");
	check(loop.wait(50) == 0, "3. writeFd sem POLLOUT: nada pronto");
	loop.setPollOut(writeFd, true);
	check(loop.wait(100) == 1, "3. setPollOut(true): wait devolve 1");
	ev = findEvent(loop.getReady(), writeFd);
	check(ev != NULL && ev->_writable, "3. setPollOut(true): evento writable");
	loop.setPollOut(writeFd, false);
	check(loop.wait(50) == 0, "3. setPollOut(false): nada pronto de novo");

	// ---- 4. setPollIn ----
	write(writeFd, "y", 1);
	loop.setPollIn(readFd, false);
	check(loop.wait(50) == 0, "4. setPollIn(false): dado pendente NÃO aparece");
	loop.setPollIn(readFd, true);
	check(loop.wait(100) == 1, "4. setPollIn(true): dado aparece");
	drainOne(readFd);

	// ---- 5. removeFd apaga evento pronto ----
	write(writeFd, "z", 1);
	check(loop.wait(100) == 1, "5. dado pendente: wait devolve 1");
	check(findEvent(loop.getReady(), readFd) != NULL, "5. evento do readFd está em getReady");
	loop.removeFd(readFd);
	check(findEvent(loop.getReady(), readFd) == NULL, "5. removeFd apagou o evento pronto desse fd");
	check(!loop.hasFd(readFd), "5. removeFd: hasFd devolve false");
	check(loop.size() == 1, "5. removeFd: size == 1");
	drainOne(readFd);

	// ---- 6. addFdPoll duplicado ----
	loop.addFdPoll(writeFd);
	check(loop.size() == 1, "6. addFdPoll do mesmo fd não duplica");

	// ---- 7. POLLHUP ----
	loop.removeFd(writeFd);
	loop.addFdPoll(readFd);
	close(writeFd);
	check(loop.wait(100) >= 1, "7. fechar a ponta de escrita: wait acorda");
	ev = findEvent(loop.getReady(), readFd);
	check(ev != NULL && ev->_error, "7. evento com error (POLLHUP) no readFd");

	// ---- 8. fd inexistente ----
	loop.removeFd(9999);
	check(true, "8. removeFd(9999) não quebrou");

	close(readFd);
	std::cout << "\n" << g_total << " testes, " << g_failed << " falharam" << std::endl;
	return g_failed == 0 ? 0 : 1;
}
 