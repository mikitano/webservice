/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42sp.org.br>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 16:55:09 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/12 02:38:15 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//test LocationConfig
/*
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
*/

//test ServerConfig e LocationConfig

#include "ServerConfig.hpp"
#include <iostream>

int main()
{
    ServerConfig server;

    // Server configuration
    server.setPort(8080);
    server.setClientMaxBodySize(10485760);

    server.addServerName("localhost");

    server.addErrorPage(404, "./www/errors/404.html");
    server.addErrorPage(500, "./www/errors/500.html");

    // First location: /
    LocationConfig home;

    home.setPath("/");
    home.setRoot("./www");
    home.addIndex("index.html");
    home.addAllowedMethod("GET");

    server.addLocation(home);

    // Second location: /uploads
    LocationConfig uploads;

    uploads.setPath("/uploads");
    uploads.setRoot("./www");
    uploads.addAllowedMethod("GET");
    uploads.addAllowedMethod("POST");
    uploads.addAllowedMethod("DELETE");
    uploads.setUploadStore("./www/uploads");

    server.addLocation(uploads);

    // Display server configuration
    std::cout << "========== SERVER ==========" << std::endl;

    std::cout << "Port: " << server.getPort() << std::endl;
    std::cout << "Body size: "
              << server.getClientMaxBodySize() << std::endl;

    std::cout << "\nServer names:" << std::endl;

    for (std::size_t i = 0; i < server.getServerNames().size(); i++)
        std::cout << "- " << server.getServerNames()[i] << std::endl;

    std::cout << "\nError pages:" << std::endl;

    std::cout << "404: " << server.getErrorPage(404) << std::endl;
    std::cout << "500: " << server.getErrorPage(500) << std::endl;
    std::cout << "403: " << server.getErrorPage(403) << std::endl;

    // Display locations
    std::cout << "\n========== LOCATIONS ==========" << std::endl;

    const std::vector<LocationConfig>& locations = server.getLocations();

    for (std::size_t i = 0; i < locations.size(); i++)
    {
        std::cout << "\nPath: " << locations[i].getPath() << std::endl;
        std::cout << "Root: " << locations[i].getRoot() << std::endl;

        std::cout << "Allowed methods:" << std::endl;

        if (locations[i].isMethodAllowed("GET"))
            std::cout << "- GET" << std::endl;

        if (locations[i].isMethodAllowed("POST"))
            std::cout << "- POST" << std::endl;

        if (locations[i].isMethodAllowed("DELETE"))
            std::cout << "- DELETE" << std::endl;

        std::cout << "Upload store: "
                  << locations[i].getUploadStore() << std::endl;
    }

        // Copy constructor test
    ServerConfig copy(server);

    // Assignment operator test
    ServerConfig another;
    another = server;

    std::cout << "\n========== COPY TEST ==========" << std::endl;

    // Modify the copies
    copy.setPort(8081);
    another.setPort(8082);

    LocationConfig newLocation;
    newLocation.setPath("/test");

    copy.addLocation(newLocation);
    another.addServerName("site2.local");

    // Compare original and copies
    std::cout << "Original port: " << server.getPort() << std::endl;
    std::cout << "Copy port: " << copy.getPort() << std::endl;
    std::cout << "Another port: " << another.getPort() << std::endl;

    std::cout << "Original locations: "
              << server.getLocations().size() << std::endl;

    std::cout << "Copy locations: "
              << copy.getLocations().size() << std::endl;

    std::cout << "Original server names: "
              << server.getServerNames().size() << std::endl;

    std::cout << "Another server names: "
              << another.getServerNames().size() << std::endl;

    return (0);
}