typedef enum {
    false,
    true
} bool;

typedef struct {
    const char *name;
    const char *template;
} Service;

typedef struct {
    const char *entity;
    const char *replacement;
} Dictionary;

void string_replace(char *str, char find, char replace);
void string_remove(char *str, const char *rem);
void string_trim(char *str);
void string_remove_redundent_spaces(char *str);
void dictionary_replace(char *str, const Dictionary *dict, int size);
int m_pow(int x, int y);
int hex_value(char c);
void ucs_to_utf8(char *ucs);
void unicode_decode(char *str);
