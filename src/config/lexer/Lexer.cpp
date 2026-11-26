/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/14 12:16:50 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/25 00:48:20 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/lexer/Lexer.hpp"
#include <stdexcept>

Lexer::Lexer()
	: _line(1),
	  _tokenLine(1) {}

Lexer::Lexer(const Lexer& copy)
{
	*this = copy;
}

Lexer& Lexer::operator=(const Lexer& rhs)
{
	if (this != &rhs)
	{
		_tokens = rhs._tokens;
		_currentToken = rhs._currentToken;
		_line = rhs._line;
		_tokenLine = rhs._tokenLine;
	}
	return (*this);
}

Lexer::~Lexer() {}

// Token helpers

void Lexer::flushToken()
{
	if (!_currentToken.empty())
	{
		_tokens.push_back(Token(WORD, _currentToken, _tokenLine));
		_currentToken.clear();
	}
}

// Handlers

void Lexer::handleWord(const std::string& content, std::size_t& i)
{
	while (i < content.size())
	{
		char c = content[i];

		if (c == ' ' || c == '\t' || c == '\r' ||
			c == '\v' || c == '\f' || c == '\n' ||
			c == '#' || c == '"' || c == '\'' ||
			c == '{' || c == '}' || c == ';')
			break;

		if (_currentToken.empty())
			_tokenLine = _line;

		_currentToken += c;
		i++;
	}
}

void Lexer::handleQuote(const std::string& content, std::size_t& i)
{
	char quote = content[i];
	std::size_t start = i + 1;

	if (!_currentToken.empty())
		throw std::runtime_error("Lexer error: quote must start a new token");

	_tokenLine = _line;
	i++;
	while (i < content.size() && content[i] != quote)
	{
		if (content[i] == '\n')
			_line++;

		_currentToken += content[i];
		i++;
	}
	if (i >= content.size())
		throw std::runtime_error("Lexer error: unclosed quote");

	if (i == start)
		throw std::runtime_error("Lexer error: empty quote");
	i++;

	if (i < content.size())
	{
		char next = content[i];
		if (next != ' ' && next != '\t' && next != '\r' &&
			next != '\v' && next != '\f' && next != '\n' &&
			next != '#' && next != '{' && next != '}' &&
			next != ';')
			throw std::runtime_error("Lexer error: invalid character after quote");
	}
	flushToken();
}

void Lexer::handleComment(const std::string& content, std::size_t& i)
{
	flushToken();
	while (i < content.size() && content[i] != '\n')
		i++;
}

void Lexer::handleSpecial(char c)
{
	flushToken();
	if (c == '{')
		_tokens.push_back(Token(LBRACE, "{", _line));
	else if (c == '}')
		_tokens.push_back(Token(RBRACE, "}", _line));
	else if (c == ';')
		_tokens.push_back(Token(SEMICOLON, ";", _line));
}

// Main function

std::vector<Token> Lexer::tokenize(const std::string& content)
{
	std::size_t i = 0;
	_tokens.clear();
	_currentToken.clear();
	_line = 1;
	_tokenLine = 1;

	while (i < content.size())
	{
		char c = content[i];
		if (c == ' ' || c == '\t' || c == '\r' ||
			c == '\v' || c == '\f')
		{
			flushToken();
			i++;
		}
		else if (c == '\n')
		{
			flushToken();
			_line++;
			i++;
		}
		else if (c == '#')
			handleComment(content, i);
		else if (c == '"' || c == '\'')
			handleQuote(content, i);
		else if (c == '{' || c == '}' || c == ';')
		{
			handleSpecial(c);
			i++;
		}
		else
			handleWord(content, i);
	}
	flushToken();
	return (_tokens);
}

//ATENCÃO: Revisar casos adicionais de aspas e caracteres especiais