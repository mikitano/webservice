#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <string>

class MimeTypes
{
	public:
		static std::string	fromPath(const std::string& path);

	private:
		MimeTypes();
};

#endif
