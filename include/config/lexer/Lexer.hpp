/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/14 12:16:36 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/14 18:48:13 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <cstddef>
#include "config/lexer/Token.hpp"

class Lexer {
	private:
		std::vector<Token> _tokens;
		std::string _currentToken;
		std::size_t _line;
		std::size_t _tokenLine;

		void flushToken();

		void handleWord(const std::string& content, std::size_t& i);
		void handleQuote(const std::string& content, std::size_t& i);
		void handleComment(const std::string& content, std::size_t& i);
		void handleSpecial(char c);

	public:
		Lexer();
		Lexer(const Lexer& copy);
		Lexer& operator=(const Lexer& rhs);
		~Lexer();

		std::vector<Token> tokenize(const std::string& content);
};

#endif