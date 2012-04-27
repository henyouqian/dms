#ifndef __LW_HTTP_CLIENT_H__
#define __LW_HTTP_CLIENT_H__


namespace lw{
	
	class HTTPClient;
	class HTTPMsg{
	public:
		HTTPMsg(const char* route, lw::HTTPClient* pClient, bool useHTTPS = false);
        virtual ~HTTPMsg();
        void addParam(const char* param);
		void send();
		virtual void onRespond(){}
		HTTPClient* getClient(){
			return _pClient;
		}
        std::string& getBuff(){
            return _buff;
        }
        void* _pObjCCallback;

	protected:
		std::string _buff;
		HTTPClient* _pClient;
        bool _useHTTPS;

	friend class HTTPClient;
	};

	class HTTPClient{
	public:
        HTTPClient(const char* host);
		~HTTPClient();
		void sendMsg(HTTPMsg* pMsg, bool useHTTPS);
        void enableHTTPS(bool b){
            _httpsEnable = b;
        }

	private:
		std::string _strHost;
        bool _httpsEnable;
	};
    
} //namespace lw

#endif //__LW_HTTP_CLIENT_H__