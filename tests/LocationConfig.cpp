/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 14:44:50 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/11 18:30:30 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include <cstddef>

LocationConfig::LocationConfig()
	: _autoindex(false),
	  _hasReturn(false),
	  _returnCode(0) {}


LocationConfig::LocationConfig(const LocationConfig& copy)
{
	*this = copy;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& rhs)
{
	if (this != &rhs)
	{
		_path = rhs._path;
		_root = rhs._root;
		_index = rhs._index;
		_allowedMethods = rhs._allowedMethods;
		_autoindex = rhs._autoindex;
		_uploadStore = rhs._uploadStore;
		_cgi = rhs._cgi;
		_hasReturn = rhs._hasReturn;
		_returnCode = rhs._returnCode;
		_returnUrl = rhs._returnUrl;
	}
	return (*this);
}

LocationConfig::~LocationConfig() {}

bool LocationConfig::isMethodAllowed(const std::string& method) const
{
	for (std::size_t i = 0; i < _allowedMethods.size(); i++)
	{
		if (_allowedMethods[i] == method)
			return true;
	}
	return false;
}

bool LocationConfig::hasReturn() const
{
	return (_hasReturn);
}

// getters

const std::string& LocationConfig::getPath() const
{
	return (_path);
}

const std::string& LocationConfig::getRoot() const
{
	return (_root);
}

const std::vector<std::string>& LocationConfig::getIndex() const
{
	return (_index);
}

const std::vector<std::string>& LocationConfig::getAllowedMethods() const
{
	return (_allowedMethods);
}

bool LocationConfig::getAutoindex() const
{
	return (_autoindex);
}

const std::string& LocationConfig::getUploadStore() const
{
	return (_uploadStore);
}

const std::map<std::string, std::string>& LocationConfig::getCgi() const
{
	return (_cgi);
}

int LocationConfig::getReturnCode() const
{
	return (_returnCode);
}

const std::string& LocationConfig::getReturnUrl() const
{
	return (_returnUrl);
}

// setters

void LocationConfig::setPath(const std::string& path)
{
	_path = path;
}

void LocationConfig::setRoot(const std::string& root)
{
	_root = root;
}

void LocationConfig::setAutoindex(bool autoindex)
{
	_autoindex = autoindex;
}

void LocationConfig::setUploadStore(const std::string& path)
{
	_uploadStore = path;
}

void LocationConfig::setReturn(int code, const std::string& url)
{
	_hasReturn = true;
	_returnCode = code;
	_returnUrl = url;
}

// adders

void LocationConfig::addIndex(const std::string& index)
{
	_index.push_back(index);
}

void LocationConfig::addAllowedMethod(const std::string& method)
{
	_allowedMethods.push_back(method);
}

void LocationConfig::addCgi(const std::string& extension, const std::string& interpreter)
{
	_cgi[extension] = interpreter;
}