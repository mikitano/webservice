/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 10:51:44 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/24 18:39:49 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/parser/Parser.hpp"
#include <stdexcept>
#include <sstream>

std::vector<std::string> Parser::readArguments()
{
	std::vector<std::string> arguments;
	while (hasNext())
	{
		const Token& token = peek();
		if (token.getType() == SEMICOLON)
		{
			if (arguments.empty())
				error("Directive requires at least one argument",
					  token.getLine());
			advance();
			return arguments;
		}
		if (token.getType() != WORD)
		{
			error("Unexpected token '" + token.getValue() + "', expected an argument or ';'", token.getLine());
			return arguments;
		}
		arguments.push_back(token.getValue());
		advance();
	}
	std::size_t line = 0;
	if (!_tokens.empty())
		line = _tokens.back().getLine();

	error("Expected ';' before end of file", line);
	return arguments;
}

// Returns the current token without advancing

const Token& Parser::peek() const 
{
	if (!hasNext())
		error("Unexpected end of file", 0);
	return (_tokens[_position]);
}

// Advances to the next token

void Parser::advance() {
	if (!hasNext())
		error("Unexpected end of file", 0);
	_position++;
}

// Checks if there are tokens left to read

bool Parser::hasNext() const {
	return (_position < _tokens.size());
}

// Checks if the current token has the expected type

void Parser::expect(TokenType type, const std::string& description)
{
	if (!hasNext())
		error("Expected " + description + ", but reached end of file", 0);

	if (peek().getType() != type)
	{
		std::string message = "Expected " + description + ", found '" + peek().getValue() + "'";
		error(message, peek().getLine());
	}
	advance();
}

// Centralizes parser error messages

void Parser::error(const std::string& message, std::size_t line) const
{
	std::stringstream ss;

	if (line > 0)
		ss << "Parser error at line " << line << ": ";
	else
		ss << "Parser error: ";
	ss << message;
	throw std::runtime_error(ss.str());
}

// Public

Parser::Parser() : _position(0) {}

Parser::Parser(const std::vector<Token>& tokens) : _tokens(tokens), _position(0) {}

Config Parser::parse()
{
	Config config;
	while (hasNext())
	{
		if (peek().getType() != WORD || peek().getValue() != "server")
		{
			error("Expected 'server'", peek().getLine());
		}
		advance();
		ServerConfig server;
		parseServer(server);

		const std::vector<ServerConfig>& cServers = config.getServers();
		for (std::size_t i = 0; i < cServers.size(); i++)
		{
			if (cServers[i].getPort() == server.getPort())
			{
				error("Duplicate server port", peek().getLine());
			}
		}
		config.addServer(server);
	}
	if (config.getServers().empty())
		error("Configuration must contain at least one server", 0);
	return config;
}