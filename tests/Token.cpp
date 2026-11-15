/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Token.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/14 11:13:06 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/14 18:56:16 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Token.hpp"

Token::Token()
	: _type(WORD),
	  _value(""),
	  _line(1) {}

Token::Token(TokenType type, const std::string& value, std::size_t line)
	: _type(type),
	  _value(value),
	  _line(line) {}

Token::Token(const Token& copy)
{
	*this = copy;
}

Token& Token::operator=(const Token& rhs)
{
	if (this != &rhs)
	{
		_type = rhs._type;
		_value = rhs._value;
		_line = rhs._line;
	}
	return (*this);
}

Token::~Token() {}

// Getters

TokenType Token::getType() const
{
	return (_type);
}

const std::string& Token::getValue() const
{
	return (_value);
}

std::size_t Token::getLine() const
{
	return (_line);
}