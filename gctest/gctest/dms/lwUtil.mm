#include "lwUtil.h"

#ifdef __APPLE__
#	include <CoreFoundation/cfDate.h>
#endif

namespace lw{

	void srand(){
#ifdef WIN32
		std::srand(GetTickCount());
#else //ifdef __APPLE__
		std::srand(CFAbsoluteTimeGetCurrent());
#endif
	}

} //namespace lw

#ifdef WIN32
W2UTF8::W2UTF8(const wchar_t* w){
	_size = WideCharToMultiByte(CP_UTF8, NULL, w, -1, NULL, 0, NULL, NULL);
	_buffer = new char[_size];
	WideCharToMultiByte(CP_UTF8, NULL, w, -1, _buffer, _size, NULL, NULL);
}
W2UTF8::~W2UTF8(){
	delete [] _buffer;
}
int W2UTF8::size(){
	return _size;
}
char* W2UTF8::data(){
	return _buffer;
}

W2UTF8::operator const char*(){
	return _buffer;
}

UTF82W::UTF82W(const char* c){
	_size = MultiByteToWideChar(CP_UTF8, NULL, c, -1, NULL, 0);
	_buffer = new WCHAR[_size];
	MultiByteToWideChar(CP_UTF8, NULL, c, -1, _buffer, _size);
}
UTF82W::~UTF82W(){
	delete [] _buffer;
}
int UTF82W::size(){
	return _size;
}
wchar_t* UTF82W::data(){
	return _buffer;
}

UTF82W::operator const wchar_t*(){
	return _buffer;
}
#endif //#ifdef WIN32

#ifdef __APPLE__
W2UTF8::W2UTF8(const wchar_t* w){
	_str = [[NSString alloc]initWithBytes:w length:wcslen(w)*4 encoding:NSUTF32LittleEndianStringEncoding ];
}
W2UTF8::~W2UTF8(){
	[_str release];
}
int W2UTF8::size(){
	return [_str length];
}
const char* W2UTF8::data(){
	return [_str UTF8String];
}

W2UTF8::operator const char*(){
	return [_str UTF8String];
}

UTF82W::UTF82W(const char* c){
    _str = [[NSString alloc]initWithUTF8String: c];
}
UTF82W::~UTF82W(){
	[_str release];
}
int UTF82W::size(){
	return [_str length];
}

const wchar_t* UTF82W::data(){
	return (const wchar_t*)[_str cStringUsingEncoding:NSUTF16LittleEndianStringEncoding];
}

UTF82W:: operator const wchar_t*(){
	return (const wchar_t*)[_str cStringUsingEncoding:NSUTF16LittleEndianStringEncoding];
}


#endif //#ifdef __APPLE__