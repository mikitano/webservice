# Nath, como usar a minha parte (HTTP, respostas e CGI)

Explicação para Nath. Esse arquivo foi criado Por Ia para explicar aquilo que desenvolvi e como integrar com a sua parte da melhor forma a maior parte dele foi feito por ia eu só Editei nesse arquivo aquilo que achei necessário e se tiver dúvidas eu vou explicar presencialmente eu só criei assim para conseguir explicar dar melhor forma e conseguir lembrar de tudo pois é muita coisa.
 Vou te explicar como eu pensei a minha parte e como você pode encaixá-la no seu `Server`.
A ideia é você implementar o loop do jeito que preferir. Aqui eu só conto o que a minha parte espera
receber e o que ela devolve.

---

## A ideia geral

Quando comecei, pensei: "a rede é da Nath; eu não posso fazer `recv`, `send` nem mexer no poll".
Então montei a minha parte como uma **caixa que transforma bytes**:

- você me entrega os bytes que leu do socket;
- eu entendo a requisição, decido a resposta e deixo os bytes prontos para você enviar;
- se a resposta precisar de um CGI, eu te aviso quais fds de pipe precisam entrar no **seu** poll.

Essa caixa é a classe **`HttpSession`**. É a única que você precisa chamar: uma por cliente.
Todo o resto (parser, arquivos, upload, DELETE, CGI) fica escondido dentro dela.

```
   seu loop                         HttpSession
   ────────                         ───────────
   recv() ──── bytes ─────────────▶ onReceive()
                                       │ entende a requisição
                                       │ monta a resposta (ou inicia um CGI)
   send() ◀─── getOutput() ────────────┘
   onSent(n) ─────────────────────▶ apaga o que já foi enviado
```

---

## Antes de tudo: o que já está pronto e o que é seu

**Você não precisa implementar nenhuma função da minha classe.** Tudo o que está na `HttpSession` já está
feito e testado. O seu trabalho é o loop do `poll` (que já seria seu de qualquer jeito) e, dentro dele,
**chamar** as minhas funções no momento certo.

### O que eu já fiz (você só chama)

| Função minha | O que ela já faz por dentro |
|---|---|
| `onReceive(buf, n)` | interpreta a requisição (mesmo em pedaços), decide a resposta, inicia o CGI |
| `onPeerClosed()` | trata o cliente que parou de enviar |
| `hasOutput()` / `getOutput()` / `onSent(n)` | guarda a resposta e controla o envio parcial |
| `shouldClose()` | diz se a conexão deve fechar (keep-alive, erro, `Connection: close`) |
| `getCgiStdinFd()` / `getCgiStdoutFd()` | diz quais pipes do CGI você precisa vigiar |
| `onCgiEvent(...)` | lê/escreve nos pipes e monta a resposta do CGI (inclusive 502 e 504) |
| `checkTimeout()` | mata um CGI travado e responde 504 |
| `onClientTimeout()` | prepara o 408 quando a requisição ficou pela metade |
| destrutor (`delete`) | mata o CGI, fecha os pipes e faz o `waitpid` (sem zumbi) |

O `RequestHandler`, o `HttpRequest`, o `HttpResponse` e todo o CGI também já estão prontos. Você nem precisa
abrir esses arquivos.

### O que é seu (a parte de rede)

1. **Accept:** aceitar o cliente e criar a sessão com `new HttpSession(...)`.
2. **Leitura do cliente:** fazer o `recv` e chamar `onReceive` ou `onPeerClosed`.
3. **Escrita no cliente:** fazer o `send` com `getOutput()` e chamar `onSent`.
4. **Controle do poll:** ligar e desligar o POLLOUT conforme `hasOutput()`, e colocar e tirar os fds do CGI
   conforme os dois getters.
5. **Eventos dos pipes:** quando o poll avisar um fd de CGI, chamar `onCgiEvent` na sessão dona daquele fd.
6. **Timeouts e fechamento:** chamar `checkTimeout` e `onClientTimeout`, e fechar o cliente quando
   `shouldClose()` for verdadeiro (primeiro tirar os pipes do poll, depois `delete`).

O nome dessas funções e a organização são escolha sua: pode ficar tudo no `run()` ou dividido em várias
funções.

### O desenho

Como o seu loop e a minha sessão conversam:

```
                        SEU LOOP (Server)                     MINHA PARTE (HttpSession)
                        ─────────────────                     ─────────────────────────

  listener pronto ──▶ accept() ─────────────────────────────▶ new HttpSession(config, porta,
                        │                                                     info, handler)
                        │
                        ▼
              ┌──────── poll() ◀────────────────────────────────────────────────────┐
              │         │                                                           │
              │         ├─ cliente POLLIN ──▶ recv()                                │
              │         │                      ├─ n > 0  ─────────────▶ onReceive(buf, n)
              │         │                      └─ n == 0 ─────────────▶ onPeerClosed()
              │         │                                                           │
              │         ├─ cliente POLLOUT ─▶ send(getOutput()) ──────▶ onSent(n)   │
              │         │                                                           │
              │         ├─ pipe do CGI ─────────────────────────────▶ onCgiEvent(fd, ...)
              │         │                                                           │
              │         └─ toda volta ──────────────────────────────▶ checkTimeout()
              │                               cliente parado ───────▶ onClientTimeout()
              │                                                                     │
              │   DEPOIS DE QUALQUER CHAMADA, você pergunta à sessão:               │
              │                                                                     │
              │     hasOutput()?        sim → POLLOUT ligado   não → desligado      │
              │     getCgiStdinFd()     >= 0 → no poll (POLLOUT)  -1 → fora do poll │
              │     getCgiStdoutFd()    >= 0 → no poll (POLLIN)   -1 → fora do poll │
              │     shouldClose()?      sim → tira os pipes do poll, delete, close  │
              │                                                                     │
              └─────────────────────────────────────────────────────────────────────┘
```

E o caminho de uma requisição com CGI, do começo ao fim:

```
 cliente            seu loop                    HttpSession                   script (python)
    │                   │                            │                               │
    │── POST /a.py ────▶│ recv ─────▶ onReceive ────▶│ requisição completa           │
    │                   │                            │── fork + execve ─────────────▶│
    │                   │◀─ getCgiStdinFd/StdoutFd ──│ (os pipes aparecem)           │
    │                   │ põe os pipes no poll       │                               │
    │                   │                            │                               │
    │                   │ poll: stdin pronto ───────▶│ onCgiEvent → write(body) ────▶│
    │                   │ poll: stdout pronto ──────▶│ onCgiEvent → read(saída) ◀────│
    │                   │                            │ script terminou: monta a      │
    │                   │◀─ hasOutput() = true ──────│ resposta, fecha os pipes (-1) │
    │                   │ tira os pipes do poll      │                               │
    │◀── 200 OK ────────│ send ─────▶ onSent ───────▶│                               │
```

---

## 1. Criando a sessão

Pensei que tudo o que a sessão precisa saber nasce no `accept`:

```cpp
#include "http/HttpSession.hpp"

RequestHandler handler(5);   // UM só para o servidor todo (5 = segundos que um CGI pode rodar)

// logo depois do accept:
HttpSession* sessao = new HttpSession(config, portaDoListener, client->getInfo(), handler);
```

- **`config`**: o `Config` inteiro do B, não só a lista de servers. Eu preciso dele para escolher o
  server certo pela porta + `Host` (dois servers podem dividir a porta 8080). Como eu guardo ponteiros
  para dentro dele, o `Config` precisa viver enquanto o servidor roda.
- **`portaDoListener`**: a porta do socket que fez o `accept`. É ela que me diz qual grupo de servers olhar.
- **`client->getInfo()`**: o seu `ClientInfo`. Eu uso o IP para o `REMOTE_ADDR` do CGI. Guardo só uma
  referência, então o `Client` precisa viver pelo menos tanto quanto a sessão.
- **`handler`**: o `RequestHandler` não guarda estado de cliente nenhum, por isso um só basta para todos.

---

## 2. Recebendo dados

Quando o poll disser que o cliente tem algo para ler, você faz o seu `recv` e me entrega:

```cpp
ssize_t n = recv(fd, buf, sizeof(buf), 0);

if (n > 0)
    sessao->onReceive(buf, n);
else if (n == 0)
    sessao->onPeerClosed();
else
    /* erro: fechar o cliente (sem olhar errno) */;
```

Como eu pensei:

- **Pode me mandar pedaços de qualquer tamanho.** A requisição chega aos poucos pela rede (às vezes
  `"GET /ind"` e depois o resto), então o meu parser guarda o que já chegou e continua de onde parou.
  Você não precisa ter buffer de leitura nem procurar `\r\n\r\n`: só me entregue o que leu.
- **Várias requisições juntas (pipelining) também funcionam.** Se o cliente mandar três de uma vez, eu
  respondo as três, na ordem.
- **Por que existe `onPeerClosed()`:** quando o `recv` devolve 0, às vezes o cliente fechou só o lado
  dele de escrita, mas ainda quer a resposta (o `nc -q` faz isso). Por isso eu não fecho na hora: eu
  respondo o que já chegou completo e marco a conexão para fechar depois do envio. Nesse momento
  vale a pena você parar de pedir POLLIN para esse fd.

---

## 3. Enviando a resposta

Eu nunca escrevo no socket. Quando tenho algo pronto, `hasOutput()` fica verdadeiro e os bytes ficam em
`getOutput()`:

```cpp
if (sessao->hasOutput())
{
    const std::string& saida = sessao->getOutput();
    ssize_t n = send(fd, saida.data(), saida.size(), 0);

    if (n <= 0)
        /* fechar o cliente */;
    else
        sessao->onSent(n);      // eu apago só o que foi enviado
}
```

Como eu pensei:

- O `send` pode enviar só uma parte (arquivo grande, cliente lento). Por isso você me diz quantos bytes
  foram com `onSent(n)`, e eu guardo o resto para a próxima vez. Você não precisa de buffer de envio.
- A minha sugestão é deixar o **POLLOUT ligado só enquanto `hasOutput()` for verdadeiro**. Assim o poll não
  fica acordando à toa.

---

## 4. Quando fechar

Quem decide isso sou eu, porque depende do HTTP. Basta você olhar `shouldClose()`:

```cpp
if (sessao->shouldClose())
    /* fechar o cliente */;
```

Ele fica verdadeiro depois que **tudo** foi enviado e a conexão não deve continuar: o cliente pediu
`Connection: close`, é HTTP/1.0 sem keep-alive, deu erro de requisição (400, 413...), deu timeout ou o
cliente fechou. Se for keep-alive, ele continua falso e a mesma conexão recebe a próxima requisição.

---

## 5. O CGI (a parte que mais conversa com o seu poll)

O subject exige **um único poll para tudo**, inclusive os pipes do CGI. Então pensei assim: eu crio o
processo e os pipes, mas **quem vigia os pipes é você**.

Quando uma requisição for para um script, a sessão passa a ter até dois fds:

| fd | o que é | coloque no poll com |
|---|---|---|
| `getCgiStdinFd()` | onde eu escrevo o body para o script | **POLLOUT** |
| `getCgiStdoutFd()` | de onde eu leio a saída do script | **POLLIN** |

Quando o poll avisar algum desses fds, você me repassa o evento:

```cpp
sessao->onCgiEvent(fd, revents & POLLIN, revents & POLLOUT, revents & (POLLERR | POLLHUP));
```

Eu faço **um** `read` ou `write` nesse fd e pronto. Quando o script termina, eu monto a resposta e ela
aparece em `hasOutput()`, como qualquer outra.

Três detalhes importantes, que vêm de como eu pensei a vida dos pipes:

1. **Os fds mudam sozinhos.** Um CGI começa dentro de `onReceive()`, o pipe de entrada fecha quando
   termino de mandar o body e os dois somem quando o script acaba. Quando um fd fecha, o getter passa
   a devolver `-1`. Por isso, **depois de qualquer chamada à sessão**, confira de novo os dois getters:
   fd novo entra no poll, fd que virou `-1` sai do poll.
2. **Tire os pipes do poll antes do `delete`.** Se o cliente cair no meio de um CGI, `delete sessao` mata
   o script, fecha os pipes e faz o `waitpid` (sem zumbi). Como os fds são fechados ali dentro, eles
   precisam sair do poll antes.
3. **Cuidado com números de fd reaproveitados.** Se um fd fecha e o mesmo número volta num `accept` ou
   num `pipe` na mesma volta do loop, o evento que ainda está na lista do `getReady()` é do fd antigo.
   Vale pular esse evento naquela volta, senão alguém faz `read` sem o poll ter liberado.

E, a cada volta do loop, chame `sessao->checkTimeout()`. É assim que eu descubro um script travado,
mato o processo e respondo 504.

### Por que eu crio os pipes e você vigia

Pode parecer estranho os pipes serem criados por mim e vigiados por você. Eu fiz assim por dois motivos.

**1. O subject só permite um `poll` no programa inteiro.**
> "usar apenas 1 poll() (ou equivalente) para todas as operações de I/O entre os clientes e o servidor"
> "Você nunca deve fazer uma operação de leitura ou escrita sem passar por poll()"

Os pipes do CGI também são I/O. O único `poll` do programa é o do seu loop (o `LoopingEvent`), então os
pipes têm que entrar nele. Se eu vigiasse os pipes sozinho, teria que escolher entre:
- um segundo `poll` só meu, que o subject proíbe;
- ler os pipes sem `poll`, que dá nota 0;
- esperar o script terminar com um `read` bloqueante, que trava o servidor inteiro enquanto o script roda.

**2. Criar os pipes faz parte de executar o CGI, que é a minha parte.**
Para rodar um script eu crio os pipes, faço `fork`, ligo os pipes no stdin/stdout do filho (`dup2`), monto
as variáveis de ambiente, faço `chdir` e `execve`. Você não precisa saber nada disso.

Então a divisão ficou assim:

| Etapa | Quem | Por quê |
|---|---|---|
| criar os pipes, `fork`, `execve` | eu | é a lógica do CGI |
| vigiar os pipes no `poll` | você | o único `poll` do programa é o seu |
| `read`/`write` nos pipes | eu (`onCgiEvent`) | só quando o seu `poll` avisa que está pronto |
| ler a saída do script e montar a resposta | eu | é HTTP/CGI |
| matar o script, fechar os pipes, `waitpid` | eu | é a lógica do CGI |

É parecido com o que acontece com os sockets, só que ao contrário: no socket do cliente você cria
(`accept`), vigia e faz o `recv`, e eu só recebo os bytes. No pipe eu crio e faço o `read`/`write`, e você só
me diz **quando** pode.

Eu pensei numa alternativa: a sessão receber o seu `LoopingEvent` e se registrar sozinha no poll
(`addFdPoll`/`removeFd`). Você teria um pouco menos de trabalho, mas o meu código ficaria preso à sua
classe interna, e qualquer mudança no seu loop quebraria a minha parte. Por isso achei melhor deixar
separado: eu só mostro os fds e você decide como colocá-los no poll. Se você preferir a alternativa, me fala.

### O que acontece se os pipes não forem vigiados direito

| Se... | O que acontece |
|---|---|
| os pipes nunca entram no poll | o script fica parado esperando alguém ler a saída; a resposta nunca fica pronta e o cliente espera até o timeout do CGI (504 em toda requisição de CGI) |
| os pipes não entram no poll **e** o `checkTimeout()` não é chamado | o cliente fica pendurado para sempre, e o subject diz que uma requisição nunca pode travar indefinidamente |
| alguém faz `read`/`write` nos pipes fora do poll (ou um segundo `poll` é criado para eles) | quebra a regra do único `poll`: nota 0 |
| um fd que virou `-1` continua no poll | o fd já foi fechado; o poll devolve `POLLNVAL` sem parar e o loop gira à toa. Se o número for reaproveitado por outro fd, o evento vai para o lugar errado |
| `delete sessao` antes de tirar os pipes do poll | mesma coisa: o poll fica vigiando fds fechados |
| o evento de um pipe vai para a sessão errada | essa sessão ignora (o fd não é dela), e o CGI da sessão certa fica parado até o timeout |
| o POLLOUT do stdin do script não é pedido | o body do POST nunca chega no script; ele espera o stdin e acaba em 504 |

Para não cair em nenhum desses casos, basta seguir a regra do começo: **depois de qualquer chamada à
sessão, confira os dois getters e atualize o poll.**

---

## 6. Timeouts

Pensei em dividir assim:

- **o timeout do cliente é seu** (quanto tempo um cliente pode ficar parado). Quando estourar, chame
  `sessao->onClientTimeout()`: se havia uma requisição pela metade, eu preparo um 408; se a conexão só
  estava ociosa, eu só marco para fechar. Depois é só seguir o `shouldClose()`;
- **o timeout do CGI é meu** (`checkTimeout()`). Enquanto `sessao->isWaitingCgi()` for verdadeiro, não
  conte o cliente como parado: ele está esperando o script.

---

## 7. Resumo em uma tabela

| Quando | Chame |
|---|---|
| depois do `accept` | `new HttpSession(config, porta, client->getInfo(), handler)` |
| `recv` > 0 | `onReceive(buf, n)` |
| `recv` == 0 | `onPeerClosed()` |
| `hasOutput()` | `send(getOutput())` e depois `onSent(n)` |
| `shouldClose()` | fechar o cliente |
| fd de CGI pronto | `onCgiEvent(fd, leitura, escrita, erro)` |
| toda volta do loop | `checkTimeout()` |
| cliente parado | `onClientTimeout()` |
| cliente caiu | tirar os pipes do poll e `delete` |

E duas regras do subject que valem para nós dois: `SIGPIPE` ignorado (o seu `Signal` já faz isso) e
nada de `errno` depois de `recv`/`send`.

---

## 8. Duas coisas que eu vi no código atual

- O `server.conf` tem dois `server` na mesma porta (8080). Hoje o `init()` abre um socket por `server`,
  e o segundo `bind` vai falhar. Basta abrir **um listener por porta**: eu escolho o server pelo `Host`.
- O `Server` hoje recebe só o `std::vector<ServerConfig>`. A sessão precisa do `Config` inteiro.

---

## 9. Arquivos e Makefile

Os meus arquivos ficam em `include/http`, `include/cgi`, `src/http` e `src/cgi`. Para o Makefile:

```make
http/HttpRequest.cpp http/HttpResponse.cpp http/HttpSession.cpp http/MimeTypes.cpp \
http/MultipartParser.cpp http/RequestHandler.cpp \
cgi/CgiAdapter.cpp cgi/CgiEnv.cpp cgi/CgiHandler.cpp cgi/CgiOutputParser.cpp cgi/PathResolver.cpp
```

Compila com `-Wall -Wextra -Werror -std=c++98` assim que o B colocar as funções do `PEDIDO_PARA_B.md`.

Qualquer dúvida, me chama que eu explico.
