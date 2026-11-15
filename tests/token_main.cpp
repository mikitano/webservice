/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/14 12:11:48 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/14 12:13:25 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Token.hpp"
#include <iostream>

int main()
{
	// Test constructor with parameters
	Token token(WORD, "listen");

	std::cout << "========== TOKEN ==========" << std::endl;
	std::cout << "Value: " << token.getValue() << std::endl;
	std::cout << "Type: " << token.getType() << std::endl;

	// Test copy constructor
	Token copy(token);

	// Test assignment operator
	Token another;
	another = token;

	std::cout << "\n========== COPY TEST ==========" << std::endl;
	std::cout << "Original: " << token.getValue() << std::endl;
	std::cout << "Copy: " << copy.getValue() << std::endl;
	std::cout << "Another: " << another.getValue() << std::endl;

	// Modify the copies
	Token newToken(WORD, "8080");
	copy = newToken;

	std::cout << "\n========== MODIFICATION TEST ==========" << std::endl;
	std::cout << "Original: " << token.getValue() << std::endl;
	std::cout << "Copy: " << copy.getValue() << std::endl;

	// Test different token types
	Token openBrace(LBRACE, "{");
	Token closeBrace(RBRACE, "}");
	Token semicolon(SEMICOLON, ";");

	std::cout << "\n========== TOKEN TYPES ==========" << std::endl;
	std::cout << "Open brace: " << openBrace.getValue()
			  << " | Type: " << openBrace.getType() << std::endl;
	std::cout << "Close brace: " << closeBrace.getValue()
			  << " | Type: " << closeBrace.getType() << std::endl;
	std::cout << "Semicolon: " << semicolon.getValue()
			  << " | Type: " << semicolon.getType() << std::endl;

	return (0);
}
