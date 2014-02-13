/*           CAsyncSslSocketLayer by Tim Kosse 
          mailto: tim.kosse@filezilla-project.org)
                 Version 2.0 (2005-02-27)
-------------------------------------------------------------

Introduction
------------

CAsyncSslSocketLayer is a layer class for CAsyncSocketEx which allows you to establish SSL secured
connections. Support for both client and server side is provided.

How to use
----------

Using this class is really simple. In the easiest case, just add an instance of
CAsyncSslSocketLayer to your socket and call InitClientSsl after creation of the socket.

This class only has a couple of public functions:
- int InitSSLConnection(bool clientMode);
  This functions establishes an SSL connection. The clientMode parameter specifies wether the SSL connection 
  is in server or in client mode.
  Most likely you want to call this function right after calling Create for the socket.
  But sometimes, you'll need to call this function later. One example is for an FTP connection
  with explicit SSL: In this case you would have to call InitSSLConnection after receiving the reply
  to an 'AUTH SSL' command.
  InitSSLConnection returns 0 on success, else an error code as described below under SSL_FAILURE
- Is UsingSSL();
  Returns true if you've previously called InitClientSsl()
- SetNotifyReply(SetNotifyReply(int nID, int nCode, int result);
  You can call this function only after receiving a layerspecific callback with the SSL_VERIFY_CERT 
  id. Set result to 1 if you trust the certificate and 0 if you don't trust it.
  nID has to be the priv_data element of the t_SslCertData structure and nCode has to be SSL_VERIFY_CERT.
- CreateSslCertificate(LPCTSTR filename, int bits, unsigned char* country, unsigned char* state,
			unsigned char* locality, unsigned char* organization, unsigned char* unit, unsigned char* cname,
			unsigned char *email, CString& err);
  Creates a new self-signed SSL certificate and stores it in the given file
- SendRaw(const void* lpBuf, int nBufLen, int nFlags = 0)
  Sends a raw, unencrypted message. This may be useful after successful initialization to tell the other
  side that can use SSL.

This layer sends some layerspecific notifications to your socket instance, you can handle them in
OnLayerCallback of your socket class.
Valid notification IDs are:
- SSL_INFO 0
  There are two possible values for param2:
	SSL_INFO_ESTABLISHED 0 - You'll get this notification if the SSL negotiation was successful
	SSL_INFO_SHUTDOWNCOMPLETE 1 - You'll get this notification if the SSL connection has been shut 
                                  down successfully. See below for details.
- SSL_FAILURE 1
  This notification is sent if the SSL connection could not be established or if an existing 
  connection failed. Valid values for param2 are:
  - SSL_FAILURE_NONE 0 - Everything OK
  - SSL_FAILURE_UNKNOWN 1 - Details may have been sent with a SSL_VERBOSE_* notification.
  - SSL_FAILURE_ESTABLISH 2 - Problem during SSL negotiation
  - SSL_FAILURE_LOADDLLS 4
  - SSL_FAILURE_INITSSL 8
  - SSL_FAILURE_VERIFYCERT 16 - The remote SSL certificate was invalid
  - SSL_FAILURE_CERTREJECTED 32 - The remote SSL certificate was rejected by user
- SSL_VERBOSE_WARNING 3
  SSL_VERBOSE_INFO 4
  This two notifications contain some additional information. The value given by param2 is a 
  pointer to a null-terminated char string (char *) with some useful information.
- SSL_VERIFY_CERT 2
  This notification is sent each time a remote certificate has to be verified.
  param2 is a pointer to a t_SslCertData structure which contains some information
  about the remote certificate.
  You have to set the reply to this message using the SetNotifyReply function.
  
Be careful with closing the connection after sending data, not all data may have been sent already.
Before closing the connection, you should call Shutdown() and wait for the SSL_INFO_SHUTDOWNCOMPLETE
notification. This assures that all encrypted data really has been sent.

License
-------

Feel free to use this class, as long as you don't claim that you wrote it
and this copyright notice stays intact in the source files.
If you want to use this class in a commercial application, a short message
to tim.kosse@filezilla-project.org would be appreciated but is not required.

This product includes software developed by the OpenSSL Project
for use in the OpenSSL Toolkit. (http://www.openssl.org/)

Version history
---------------

Version 2.0:
- Add server support
- a lot of bug fixes

*/

#ifndef ASYNCSSLSOCKETLEAYER_INCLUDED
#define ASYNCSSLSOCKETLEAYER_INCLUDED

#ifndef _AFX
#define CString CStdString
#endif

#include "AsyncSocketExLayer.h"
#ifdef MPEXT
#define OPENSSL_NO_EC
#define OPENSSL_NO_ECDSA
#define OPENSSL_NO_ECDH
#endif
#include <openssl/ssl.h>

// Details of SSL certificate, can be used by app to verify if certificate is valid
struct t_SslCertData
{
	struct t_Contact
	{
		TCHAR Organization[256];
		TCHAR Unit[256];
		TCHAR CommonName[256];
		TCHAR Mail[256];
		TCHAR Country[256];
		TCHAR StateProvince[256];
		TCHAR Town[256];
		TCHAR Other[1024];
	} subject, issuer;

	struct t_validTime
	{
		//Year, Month, day, hour, minute, second
		int y,M,d,h,m,s;
	} validFrom, validUntil;

	unsigned char hash[20];

	int verificationResult;
	int verificationDepth;

	int priv_data; //Internal data, do not modify
};

class CCriticalSectionWrapper;
class CAsyncSslSocketLayer : public CAsyncSocketExLayer
{
public:
	BOOL SetCertStorage(CString file);
	CAsyncSslSocketLayer();
	virtual ~CAsyncSslSocketLayer();

	void SetNotifyReply(int nID, int nCode, int result);
	BOOL GetPeerCertificateData(t_SslCertData &SslCertData, LPCTSTR & Error);
	std::string GetTlsVersionStr();
	std::string GetCipherName();

	bool IsUsingSSL();
	int InitSSLConnection(bool clientMode, 
		CAsyncSslSocketLayer* main,
		bool sessionreuse, int minTlsVersion, int maxTlsVersion,
		void* pContext = 0);

	static bool CreateSslCertificate(LPCTSTR filename, int bits, unsigned char* country, unsigned char* state,
			unsigned char* locality, unsigned char* organization, unsigned char* unit, unsigned char* cname,
			unsigned char *email, CString& err);

	int SetCertKeyFile(const char* cert, const char* key, const char* pass, CString* error = 0);

	// Send raw text, useful to send a confirmation after the ssl connection
	// has been initialized
	int SendRaw(const void* lpBuf, int nBufLen, int nFlags = 0);

	void* GetContext() { return m_ssl_ctx; }
	
private:
	virtual void Close();
	virtual BOOL Connect(LPCTSTR lpszHostAddress, UINT nHostPort );
	virtual BOOL Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen );
	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual int Receive(void* lpBuf, int nBufLen, int nFlags = 0);
	virtual int Send(const void* lpBuf, int nBufLen, int nFlags = 0);
	virtual BOOL ShutDown( int nHow = sends );
	
	void ResetSslSession();
	void PrintSessionInfo();
	BOOL ShutDownComplete();
	int InitSSL();
	void UnloadSSL();
	void PrintLastErrorMsg();

	void TriggerEvents();

	//Will be called from the OpenSSL library
	static void apps_ssl_info_callback(const SSL *s, int where, int ret);
	static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx);
	static int pem_passwd_cb(char *buf, int size, int rwflag, void *userdata);

	bool m_bUseSSL;
	BOOL m_bFailureSent;

	//Critical section for thread synchronization
	static CCriticalSectionWrapper m_sCriticalSection;

	// Status variables
	static int m_nSslRefCount;
	BOOL m_bSslInitialized;
	int m_nShutDown;
	int m_nNetworkError;
	int m_nSslAsyncNotifyId;
	BOOL m_bBlocking;
	BOOL m_bSslEstablished;
	CString m_CertStorage;
	int m_nVerificationResult;
	int m_nVerificationDepth;
	
	static struct t_SslLayerList
	{
		CAsyncSslSocketLayer *pLayer;
		t_SslLayerList *pNext;
	} *m_pSslLayerList;

#ifndef MPEXT_NO_SSLDLL
	// Handles to the SLL libraries
	static HMODULE m_hSslDll1;
	static HMODULE m_hSslDll2;
#endif

	// SSL data
	SSL_CTX* m_ssl_ctx;	// SSL context
	static std::map<SSL_CTX *, int> m_contextRefCount;
	SSL* m_ssl;			// current session handle
	SSL_SESSION * m_sessionid;
	bool m_sessionreuse;
	CAsyncSslSocketLayer * m_Main;

	//Data channels for encrypted/unencrypted data
	BIO* m_nbio;	//Network side, sends/receives encrypted data
	BIO* m_ibio;	//Internal side, won't be used directly
	BIO* m_sslbio;	//The data to encrypt / the decrypted data has to go though this bio

	//Send buffer
	char* m_pNetworkSendBuffer;
	int m_nNetworkSendBufferLen;
	int m_nNetworkSendBufferMaxLen;

	char* m_pRetrySendBuffer;
	int m_nRetrySendBufferLen;

	bool m_mayTriggerRead;
	bool m_mayTriggerWrite;
	bool m_mayTriggerReadUp;
	bool m_mayTriggerWriteUp;

	bool m_onCloseCalled;

	char* m_pKeyPassword;

	std::string m_TlsVersionStr;
	std::string m_CipherName;
};

#define SSL_INFO 0
#define SSL_FAILURE 1
#define SSL_VERIFY_CERT 2
#define SSL_VERBOSE_WARNING 3
#define SSL_VERBOSE_INFO 4

#define SSL_INFO_ESTABLISHED 0
#define SSL_INFO_SHUTDOWNCOMPLETE 1

#define SSL_FAILURE_UNKNOWN 0
#define SSL_FAILURE_ESTABLISH 1
#ifndef MPEXT_NO_SSLDLL
#define SSL_FAILURE_LOADDLLS 2
#endif
#define SSL_FAILURE_INITSSL 4
#define SSL_FAILURE_VERIFYCERT 8
#define SSL_FAILURE_CERTREJECTED 0x10

#define SSL_VERSION_SSL2 2
#define SSL_VERSION_SSL3 3
#define SSL_VERSION_TLS10 10
#define SSL_VERSION_TLS11 11
#define SSL_VERSION_TLS12 12

void __fastcall LoadSslWindowsSystemCertificateStore(SSL_CTX * Ctx);

#endif // ASYNCSSLSOCKETLEAYER_INCLUDED
