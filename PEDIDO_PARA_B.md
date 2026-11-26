# B, o que a minha parte precisa do config

Oi! Vou te explicar como a minha parte (HTTP, respostas e CGI) usa o seu modelo e por que eu preciso
de mais três funções. Já uso quase tudo que você fez, do jeito que está.

---

## Como eu uso o config numa requisição

Pensei no caminho de uma requisição assim:

```
1. chega "GET /uploads/foto.png" com "Host: localhost" na porta 8080
2. qual server atende?   -> porta 8080 + Host "localhost"          -> Config::findServer()
3. qual location?        -> o maior prefixo que casa com o path    -> ServerConfig::matchLocation()
4. o que esse location permite? -> os getters que você já fez
```

No passo 4 eu já uso, sem mudar nada:

| Para quê | Seus getters |
|---|---|
| achar o arquivo | `getRoot()`, `getIndex()`, `getAutoindex()` |
| métodos e o header `Allow` do 405 | `isMethodAllowed()`, `getAllowedMethods()` |
| redirect | `hasReturn()`, `getReturnCode()`, `getReturnUrl()` |
| upload | `getUploadStore()` |
| CGI | `getCgi()` (extensão → interpretador) |
| limite de body | `ServerConfig::getClientMaxBodySize()` |
| páginas de erro | `ServerConfig::getErrorPage()` |
| CGI e virtual host | `getServerNames()`, `getPort()` |

Faltam os passos 2 e 3, e um detalhe do limite de body.

---

## 1. `Config::findServer(porta, host)`

**Por que eu preciso:** o `server.conf` tem dois `server` na porta 8080 (`localhost` e `outro.local`).
Quando chega uma requisição, eu preciso escolher um deles pelo header `Host`. Pensei na mesma regra do
NGINX: vale o server cujo `server_name` for igual ao `Host`; se nenhum bater, vale o **primeiro** server
daquela porta.

Eu guardo o ponteiro que você devolver, então não faço cópia nenhuma. Por isso o `Config` precisa
continuar vivo enquanto o servidor roda.

```cpp
// Config.hpp
const ServerConfig* findServer(int port, const std::string& host) const;

// Config.cpp
const ServerConfig* Config::findServer(int port, const std::string& host) const
{
	const ServerConfig* first = NULL;

	for (std::size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers[i].getPort() != port)
			continue;
		if (first == NULL)
			first = &_servers[i];
		const std::vector<std::string>& names = _servers[i].getServerNames();
		for (std::size_t j = 0; j < names.size(); j++)
			if (names[j] == host)
				return (&_servers[i]);
	}
	return (first);   // NULL se a porta não existe
}
```

---

## 2. `ServerConfig::matchLocation(path)`

**Por que eu preciso:** um path como `/uploads/foto.png` pode casar com `/` e com `/uploads`. Vale o
**prefixo mais longo** (`/uploads`). Tem uma pegadinha: `/uploads` não pode casar com `/uploadsX`,
então eu confiro se depois do prefixo vem uma `/` ou o fim do path.

```cpp
// ServerConfig.hpp
const LocationConfig* matchLocation(const std::string& path) const;

// ServerConfig.cpp
const LocationConfig* ServerConfig::matchLocation(const std::string& path) const
{
	const LocationConfig* best = NULL;
	std::size_t bestLen = 0;

	for (std::size_t i = 0; i < _locations.size(); i++)
	{
		const std::string& prefix = _locations[i].getPath();

		if (prefix.empty() || path.compare(0, prefix.size(), prefix) != 0)
			continue;
		if (prefix[prefix.size() - 1] != '/' && path.size() > prefix.size()
			&& path[prefix.size()] != '/')
			continue;
		if (best == NULL || prefix.size() > bestLen)
		{
			best = &_locations[i];
			bestLen = prefix.size();
		}
	}
	return (best);   // NULL se nenhum casar
}
```

---

## 3. `LocationConfig::getClientMaxBodySize()`

**Por que eu preciso:** rodei o `./tester` da 42 e ele pede, no mesmo server, `/post_body` com limite de
**100 bytes** e um arquivo `.bla` recebendo **100 MB**. Com o limite só no server, não dá para atender os
dois. Então pensei: se o location definir `client_max_body_size`, vale o dele; se não definir (`0`), vale
o do server.

```cpp
// LocationConfig.hpp
std::size_t getClientMaxBodySize() const;
void        setClientMaxBodySize(std::size_t size);
// membro: std::size_t _clientMaxBodySize;  (começa em 0 e é copiado no operator=)
```

---

## 4. Como eu interpreto o que o parser monta

Para a gente não se desencontrar, é assim que a minha parte lê cada coisa:

- **`root`**: igual ao NGINX, o path inteiro é anexado. Com `location /uploads { root ./www; }`,
  `/uploads/a.txt` vira `./www/uploads/a.txt`.
- **Location sem `root`** (e sem `return`): respondo 500 e não leio nada do disco, para nunca servir
  arquivo a partir da raiz `/`. Se o `root` vier no `server`, copie para os locations que não têm.
- **`allow_methods`**: sigo ao pé da letra. HEAD só funciona se estiver na lista (o tester exige 405 para
  HEAD numa rota "só GET"). Sem `allow_methods`, tudo vira 405, então sugiro o padrão `GET`.
- **`client_max_body_size`** 0 ou ausente: uso 1 MB. Valores como `10M` precisam chegar já em bytes.
- **`error_page 404 /errors/404.html`**: procuro esse arquivo no `root` do location que casar com
  `/errors/404.html`. Se o arquivo não existir, uso a minha página padrão.
- **`cgi .py /usr/bin/python3`**: se o interpretador não for executável, respondo 500.
- **Erro no `.conf`**: lança uma exceção com mensagem e o programa sai. Nunca crash.

Se você quiser mudar alguma dessas regras, me avisa que eu ajusto do meu lado.
