/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 16:55:09 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/09 17:13:44 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//main test config

#include "LocationConfig.hpp"
#include <iostream>

int main()
{
    LocationConfig location;

    location.setPath("/");
    location.setRoot("./www");

    location.addIndex("index.html");
    location.addAllowedMethod("GET");
    location.addAllowedMethod("POST");

    location.addCgi(".py", "/usr/bin/python3");

    location.setReturn(301, "/");

    std::cout << "Path: " << location.getPath() << std::endl;
    std::cout << "Root: " << location.getRoot() << std::endl;

    // if (location.isMethodAllowed("GET"))
    //     std::cout << "GET is allowed" << std::endl;

    // if (!location.isMethodAllowed("DELETE"))
    //     std::cout << "DELETE is not allowed" << std::endl;

    if (location.hasReturn())
        std::cout << "Return code: " << location.getReturnCode() << std::endl;

    std::cout << "Return URL: " << location.getReturnUrl() << std::endl;

    return (0);
}