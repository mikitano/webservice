/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42sp.org.br>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 16:55:09 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/10 22:29:49 by mkitano          ###   ########.fr       */
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

    location.setUploadStore("./www/uploads");
    location.setAutoindex(true);

    location.addCgi(".py", "/usr/bin/python3");

    std::cout << "Path: " << location.getPath() << std::endl;
    std::cout << "Root: " << location.getRoot() << std::endl;

    if (location.isMethodAllowed("GET"))
        std::cout << "GET is allowed" << std::endl;

    if (!location.isMethodAllowed("DELETE"))
        std::cout << "DELETE is not allowed" << std::endl;

    if (!location.hasReturn())
        std::cout << "No redirect configured" << std::endl;

    location.setReturn(301, "/");

    if (location.hasReturn())
    {
        std::cout << "Return code: " << location.getReturnCode() << std::endl;
        std::cout << "Return URL: " << location.getReturnUrl() << std::endl;
    }

    if (!location.getIndex().empty())
        std::cout << "Index: " << location.getIndex()[0] << std::endl;

    std::cout << "Upload: " << location.getUploadStore() << std::endl;
    std::cout << "Autoindex: " << location.getAutoindex() << std::endl;

    if (location.getCgi().find(".py") != location.getCgi().end())
        std::cout << "CGI: " << location.getCgi().find(".py")->second << std::endl;

    LocationConfig copy(location);

    LocationConfig another;
    another = location;

    std::cout << "\n=========== Copy Test ===========" << std::endl;

    another.setPath("/alterado");
    another.addAllowedMethod("DELETE");
    another.setAutoindex(false);

    std::cout << "Original path: " << location.getPath() << std::endl;
    std::cout << "Copy path: " << another.getPath() << std::endl;

    std::cout << "Original autoindex: " << location.getAutoindex() << std::endl;
    std::cout << "Copy autoindex: " << another.getAutoindex() << std::endl;

    std::cout << "Original allows DELETE: "
            << location.isMethodAllowed("DELETE") << std::endl;

    std::cout << "Copy allows DELETE: "
            << another.isMethodAllowed("DELETE") << std::endl;

    return (0);
}