/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 19:51:37 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/21 12:15:36 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>
#include <cstddef>
#include "config/lexer/Token.hpp"
#include "config/model/Config.hpp"
#include "config/model/ServerConfig.hpp"
#include "config/model/LocationConfig.hpp"

class Parser {
	private:
		std::vector<Token> _tokens;
		std::size_t _position;

		// void parseServer(Config& config);
		// void parseLocation(ServerConfig& server);

		// void parseServerDirective(ServerConfig& server, const std::string& directive);
		// void parseLocationDirective(LocationConfig& location, const std::string& directive);

		std::vector<std::string> readArguments();

		const Token& peek() const;
		void advance();
		bool hasNext() const;
		void expect(TokenType type, const std::string& description);

		void error(const std::string& message, std::size_t line) const;

	public:
		Parser();
		Parser(const std::vector<Token>& tokens);

		// Config parse();
};

#endif