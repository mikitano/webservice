#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <utility>
#include <vector>

class HttpResponse
{
	public:
		HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse& operator=(const HttpResponse& other);
		~HttpResponse();

		void	setStatus(int code);
		void	setStatus(int code, const std::string& text);
		void	setHeader(const std::string& name, const std::string& value);
		void	setBody(const std::string& body);
		void	setHeadOnly(bool headOnly);

		int					getStatus() const;
		const std::string&	getStatusText() const;
		bool				hasHeader(const std::string& name) const;
		std::string			getHeader(const std::string& name) const;
		const std::string&	getBody() const;
		bool				isHeadOnly() const;

		std::string			toString() const;

		static std::string		reasonPhrase(int code);
		static HttpResponse		makeError(int code);
		static HttpResponse		makeRedirect(int code, const std::string& location);

	private:
		static std::string	toLower(const std::string& s);
		static std::string	numToString(size_t n);
		static std::string	httpDate();

		int												_status;
		std::string										_statusText;
		std::vector<std::pair<std::string, std::string> >	_headers;
		std::string										_body;
		bool											_headOnly;
};

#endif
