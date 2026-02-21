// Copyright Coffee Stain Studios. All Rights Reserved.

#include "SMLWebSocket.h"
#include "Modules/ModuleManager.h"

// OpenSSL global init/cleanup
// UI is defined as a namespace by Unreal/Slate headers, but OpenSSL's ossl_typ.h
// typedefs 'UI' as a struct type, causing C2365 redefinition errors on MSVC.
// Push/pop the macro around the OpenSSL includes to avoid the conflict.
#pragma push_macro("UI")
#undef UI
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/crypto.h"
#pragma pop_macro("UI")

void FSMLWebSocketModule::StartupModule()
{
	// OpenSSL 1.1.0+ auto-initializes; for older versions we call the explicit init.
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
#else
	// Explicit opt-in init (safe to call multiple times; reference-counted internally).
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);
#endif
}

void FSMLWebSocketModule::ShutdownModule()
{
	// OpenSSL 1.1.0+ handles cleanup automatically; nothing to do here.
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	ERR_free_strings();
	EVP_cleanup();
#endif
}

IMPLEMENT_MODULE(FSMLWebSocketModule, SMLWebSocket)
