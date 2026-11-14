/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 18:13:33 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/14 00:04:57 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum TokenType
{
	WORD,
	LBRACE,
	RBRACE,
	SEMICOLON
};

class Token
{
private:
	TokenType _type;
	std::string _value;

public:
	Token();
	Token(TokenType type, const std::string& value);
	Token(const Token& copy);
	Token& operator=(const Token& rhs);
	~Token();

	TokenType getType() const;
	const std::string& getValue() const;
};

#endif
