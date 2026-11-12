/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42sp.org.br>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/07 00:54:37 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/10 22:30:01 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <map>
#include <string>
#include <vector>

class LocationConfig
{
	private:
		std::string							_path;
		std::string							_root;
		std::vector<std::string>			_index;
		std::vector<std::string>			_allowedMethods;
		bool								_autoindex;
		std::string							_uploadStore;
		std::map<std::string, std::string>	_cgi;

		bool								_hasReturn;
		int									_returnCode;
		std::string							_returnUrl;

	public:
		LocationConfig();
		LocationConfig(const LocationConfig& copy);
		LocationConfig& operator= (const LocationConfig& rhs);
		~LocationConfig();

		bool isMethodAllowed(const std::string& method) const;
		bool hasReturn() const;

		const std::string& getPath() const;
		const std::string& getRoot() const;
		const std::vector<std::string>& getIndex() const;
		const std::vector<std::string>& getAllowedMethods() const;
		bool getAutoindex() const;
		const std::string& getUploadStore() const;
		const std::map<std::string, std::string>& getCgi() const;
		int getReturnCode() const;
		const std::string& getReturnUrl() const;
			
		void setPath(const std::string& path);
		void setRoot(const std::string& root);
		void setAutoindex(bool autoindex);
		void setUploadStore(const std::string& path);
		void setReturn(int code, const std::string& url);

		void addIndex(const std::string& index);
		void addAllowedMethod(const std::string& method);
		void addCgi(const std::string& extension,const std::string& interpreter);
};

#endif