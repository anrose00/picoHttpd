#ifndef __HTTP_CONSTANTS__
#define __HTTP_CONSTANTS__

#define HEADER_CONTENT_TYPE "Content-Type"
#define HEADER_CONTENT_LENGTH "Content-Length"
#define HEADER_WWW_AUTHENTICATE "WWW-Authenticate"
#define HEADER_AUTHORIZATION "Authorization"
#define HEADER_TRANSFER_ENCODING "Transfer-Encoding"
#define HEADER_TRANSFER_TYPE_CHUNKED "chunked"
#define HEADER_KEEP_ALIVE "Keep-Alive"
#define HEADER_CONNECTION "Connection"
#define HEADER_VALUE_CONNECTION_KEEP_ALIVE "keep-alive"
#define HEADER_VALUE_CONNECTION_CLOSE "close"

#define MAX_RESPONSE_CODE 62
typedef struct { int code; char *value; } http_response_code;
static http_response_code HTTP_RESPONSE_CODES[MAX_RESPONSE_CODE+1] = {
	{100, "Continue"},
	{101, "Switching Protocols"},
	{102, "Processing"},
	{103, "Early Hints"},
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},
	{207, "Multi-Status"},
	{208, "Already Reported"},
	{226, "IM Used"},
	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{306, "Switch Proxy"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Timeout"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{412, "Precondition Failed"},
	{413, "Payload Too Large"},
	{414, "URI Too Long"},
	{415, "Unsupported Media Type"},
	{416, "Range Not Satisfiable"},
	{417, "Expectation Failed"},
	{418, "I'm a teapot"},
	{421, "Misdirected Request"},
	{422, "Unprocessable Entity"},
	{423, "Locked"},
	{424, "Failed Dependency"},
	{425, "Too Early"},
	{426, "Upgrade Required"},
	{428, "Precondition Required"},
	{429, "Too Many Requests"},
	{431, "Request Header Fields Too Large"},
	{451, "Unavailable For Legal Reasons"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Timeout"},
	{505, "HTTP Version Not Supported"},
	{506, "Variant Also Negotiates"},
	{507, "Insufficient Storage"},
	{508, "Loop Detected"},
	{510, "Not Extended"},
	{511, "Network Authentication Required"}
};


#endif /*__HTTP_CONSTANTS__*/