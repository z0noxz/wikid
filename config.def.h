#define LANG_POS    8
#define _PADDING    3

char language[]         = "en";
char temp_file[]        = "/tmp/__wikidtmp.XXXXXX";
const char webpage[]    = "https://xx.wikipedia.org/w/api.php"
                        "?format=json"
                        "&action=query"
                        "&prop=extracts"
                        "&exlimit=1"
                        "&explaintext"
                        "&redirects"
                        "&titles=";

/* cleanup mappings for wikipedia responses
 * NOTE: the replacement cannot be greater in size than the entity itself. If
 * it is, it'll just be ignored.
 */
const Dictionary cleanup_mappings[] = {
    /* entity       replacement */
    { "\\n",        "\n\n" },
    { "\\t",        "  " },
    { "\\\"",       "\"" },
};
