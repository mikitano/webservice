/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Token.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 18:13:33 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/14 18:37:34 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <cstddef>

enum TokenType
{
	WORD,
	LBRACE,
	RBRACE,
	SEMICOLON
};

class Token {
	private:
		TokenType _type;
		std::string _value;
		std::size_t _line;

	public:
		Token();
		Token(TokenType type, const std::string& value, std::size_t line);
		Token(const Token& copy);
		Token& operator=(const Token& rhs);
		~Token();

		TokenType getType() const;
		const std::string& getValue() const;
		std::size_t getLine() const;
};

#endif