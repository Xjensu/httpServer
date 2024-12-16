#ifndef PAGEMETHODS_SECURE_H_
#define PAGEMETHODS_SECURE_H_


extern void hash_password(const char *password, char *hash);

extern void generate_token(char *token);

#endif /* PAGEMETHODS_SECURE_H_ */
