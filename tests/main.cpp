//teste1
// #include <iostream>
// #include <vector>
// #include "Lexer.hpp"

// int main()
// {
// 	std::string content =
// 		"server {\n"
// 		"    listen 8080;\n"
// 		"    server_name localhost;\n"
// 		"    location / {\n"
// 		"        root \"./www/meu site\";\n"
// 		"        index index.html;\n"
// 		"        # comment\n"
// 		"    }\n"
// 		"}\n";

// 	Lexer lexer;
// 	std::vector<Token> tokens = lexer.tokenize(content);

// 	for (std::size_t i = 0; i < tokens.size(); i++)
// 	{
// 		std::cout << "Value: " << tokens[i].getValue()
// 				  << " | Type: " << tokens[i].getType()
// 				  << " | Line: " << tokens[i].getLine()
// 				  << std::endl;
// 	}

// 	return (0);
// }

//teste2
#include "Lexer.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <exception>

void printTokens(const std::vector<Token>& tokens)
{
	std::vector<Token>::const_iterator it;

	for (it = tokens.begin(); it != tokens.end(); ++it)
	{
		std::cout << "Value: " << it->getValue()
				  << " | Type: " << it->getType()
				  << " | Line: " << it->getLine()
				  << std::endl;
	}
}

void runTest(Lexer& lexer, const std::string& name,
			 const std::string& content, bool expectError)
{
	std::cout << "\n=== " << name << " ===" << std::endl;

	try
	{
		std::vector<Token> tokens = lexer.tokenize(content);

		if (expectError)
			std::cout << "FAIL: deveria ter gerado erro." << std::endl;
		else
		{
			std::cout << "PASS: leitura realizada." << std::endl;
			printTokens(tokens);
		}
	}
	catch (const std::exception& e)
	{
		if (expectError)
			std::cout << "PASS: erro esperado: " << e.what() << std::endl;
		else
			std::cout << "FAIL: erro inesperado: " << e.what() << std::endl;
	}
}

int main()
{
	Lexer lexer;

	runTest(lexer, "Comentarios",
		"server {\n"
		"    # comentario\n"
		"    listen 8080; # outro comentario\n"
		"}\n",
		false);

	runTest(lexer, "Aspas vazias",
		"server_name \"\";\n",
		true);

	runTest(lexer, "Aspas sem fechamento",
		"root \"./www;\n",
		true);

	runTest(lexer, "Aspas simples",
		"root './www/meu site';\n",
		false);

	std::cout << "\n=== Reutilizacao do Lexer ===" << std::endl;

	runTest(lexer, "Primeiro arquivo",
		"listen 8080;\n",
		false);

	runTest(lexer, "Segundo arquivo",
		"listen 9090;\n",
		false);

	runTest(lexer, "Aspas coladas em palavras",
		"root\"./www\";\n",
		true);

	return (0);
}
