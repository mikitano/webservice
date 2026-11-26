#include "http/MimeTypes.hpp"

#include <cstddef>

namespace
{
	struct MimeEntry
	{
		const char*	ext;
		const char*	type;
	};

	const MimeEntry	g_table[] = {
		{"html", "text/html; charset=utf-8"},
		{"htm", "text/html; charset=utf-8"},
		{"css", "text/css; charset=utf-8"},
		{"js", "text/javascript; charset=utf-8"},
		{"mjs", "text/javascript; charset=utf-8"},
		{"txt", "text/plain; charset=utf-8"},
		{"csv", "text/csv; charset=utf-8"},
		{"md", "text/markdown; charset=utf-8"},
		{"xml", "application/xml"},
		{"json", "application/json"},
		{"pdf", "application/pdf"},
		{"zip", "application/zip"},
		{"gz", "application/gzip"},
		{"tar", "application/x-tar"},
		{"wasm", "application/wasm"},
		{"png", "image/png"},
		{"jpg", "image/jpeg"},
		{"jpeg", "image/jpeg"},
		{"gif", "image/gif"},
		{"webp", "image/webp"},
		{"svg", "image/svg+xml"},
		{"ico", "image/x-icon"},
		{"bmp", "image/bmp"},
		{"mp3", "audio/mpeg"},
		{"wav", "audio/wav"},
		{"ogg", "audio/ogg"},
		{"mp4", "video/mp4"},
		{"webm", "video/webm"},
		{"woff", "font/woff"},
		{"woff2", "font/woff2"},
		{"ttf", "font/ttf"},
		{NULL, NULL}
	};
}

std::string	MimeTypes::fromPath(const std::string& path)
{
	size_t	slash = path.rfind('/');
	size_t	dot = path.rfind('.');

	if (dot == std::string::npos || (slash != std::string::npos && dot < slash)
		|| dot + 1 >= path.size())
		return "application/octet-stream";
	std::string	ext = path.substr(dot + 1);

	for (size_t i = 0; i < ext.size(); ++i)
		if (ext[i] >= 'A' && ext[i] <= 'Z')
			ext[i] = static_cast<char>(ext[i] + 32);
	for (size_t i = 0; g_table[i].ext != NULL; ++i)
		if (ext == g_table[i].ext)
			return g_table[i].type;
	return "application/octet-stream";
}
