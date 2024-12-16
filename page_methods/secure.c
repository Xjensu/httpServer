#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <openssl/evp.h> 
#include <openssl/rand.h>

extern void hash_password(const char *password, char *hash) { 
	unsigned char salt[16]; 
	unsigned char hash_output[32]; 
	RAND_bytes(salt, sizeof(salt)); 
	PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), 
			10000, EVP_sha256(), sizeof(hash_output), hash_output); 
	for (int i = 0; i < 16; i++) { 
		sprintf(hash + i * 2, "%02x", salt[i]); 
	}

	for (int i = 0; i < 32; i++) { 
		sprintf(hash + (i + 16) * 2, "%02x", hash_output[i]); 
	} 
}

extern void generate_token(char *token) { 
	unsigned char token_bytes[16]; 
	RAND_bytes(token_bytes, sizeof(token_bytes)); 
	for (int i = 0; i < 16; i++) { 
		sprintf(token + i * 2, "%02x", token_bytes[i]); 
	} 
}
