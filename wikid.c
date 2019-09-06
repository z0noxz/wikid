/*
 * Downloads and renders wikipedia pages in your terminal, with some formatting
 * Copyright (C) 2018 z0noxz, <chris@noxz.tech>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

#include "util.h"
#include "term.h"
#include "config.h"

/* macros */
#define LENGTH(x)       (sizeof(x) / sizeof(x[0]))
#define PADDING(x)      (x * _PADDING)
#define INDEX_OF(x, y)  ((int)(x - y) / sizeof(y[0]))

/* function declarations */
char *get_wiki_url(char *subject);
int download_wiki(char *url);
void print_line(int padding, const char *format, char *line);
void handle_line(char *line);
int print_wiki(char *subject);

/* variables */
const char  *usage              = "usage: wikid [-hlrsSt] <subject>";
const char  *webpage            = services[0].template;
unsigned    total_line_count    = 0;
unsigned    section_line_count  = 0;
bool        blank_line          = false;
bool        pre_heading         = true;
char        global_options      = '\0';
unsigned    terminal_width;
int         temp_file_descriptor;

/* function implementations */
char *get_wiki_url(char *subject)
{
    char *url = (char*)malloc(2048 * sizeof(char));

    strcpy(url, webpage);
    strcat(url, subject);

    memmove(url + LANG_POS, language, 2);
    string_replace(url, ' ', '_');

    return url;
}

int download_wiki(char *url)
{
    int         state = 0;
    char        error[CURL_ERROR_SIZE];
    FILE        *fptr;
    CURL        *curl;
    CURLcode    status;

    temp_file_descriptor = mkstemp(temp_file);
    if (temp_file_descriptor == -1) {
        fprintf(stderr, "%s\n", "Can't open tmp file");
        return CURLE_FAILED_INIT;
    }
    fptr = fdopen(dup(temp_file_descriptor), "w");

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        if (curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error) != CURLE_OK) {
            fprintf(stderr, "%s\n", "Failed to set an error buffer for curl");
            state = -1;
        } else if (curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK) {
            fprintf(stderr, "%s\n", "Failed to set the URL for curl");
            state = -1;
        } else if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL) != CURLE_OK) {
            fprintf(stderr, "%s\n", "Failed to set write function for curl");
            state = -1;
        } else if (curl_easy_setopt(curl, CURLOPT_WRITEDATA, fptr) != CURLE_OK) {
            fprintf(stderr, "%s\n", "Failed to set output for curl");
            state = -1;
        } else if (curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1) != CURLE_OK) {
            fprintf(stderr, "%s\n", "Failed to set follow location for curl");
            state = -1;
        } else if ((status = curl_easy_perform(curl)) && status != CURLE_OK) {
            fprintf(stderr, "%s\n", curl_easy_strerror(status));
            state = status;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    if (fptr)
        fclose(fptr);

    return state;
}

void print_line(int padding, const char *format, char *line)
{
    if (padding > 0)
        fprintf(stdout, "%*c", PADDING(padding), ' ');

    if (!((global_options >> 1) & 1U))
        fprintf(stdout, format, line);
    else
        fprintf(stdout, "%s\n", line);
}

void handle_line(char *input)
{
    char    *temp_line  = (char*)malloc(strlen(input) + 1 * (sizeof(char)));
    char    *ptr        = temp_line;
    char    *line       = NULL;
    char    *next       = NULL;
    char    heading     = 0x0;
    int     b           = 0;    /* beginning    */
    int     e           = 0;    /* end          */
    int     limit       = terminal_width - PADDING(3);

    strcpy(temp_line, input);
    string_trim(temp_line);

    /* make sure first section line contains text */
    if (*temp_line || (section_line_count > 0 && !blank_line)) {

        /* check and handle headings, (h2-h4) */
        if (*ptr++ == '=' && *ptr++ == '=' && *ptr) {
            if (*ptr++ == '=' && *ptr-- == '=') {
                string_remove(temp_line, "====");
                heading = 0x4;
            } else if (*ptr == '=') {
                string_remove(temp_line, "===");
                heading = 0x3;
            } else {
                ptr--;
                /* capitalize 'lowest' heading */
                while (*ptr) {
                    if (*ptr > 0x60 && *ptr < 0x7b)
                        *ptr -= 0x20;
                    if ((unsigned char)*(ptr - 1) == 0xc3
                    && (unsigned char)*ptr >= 0xa0
                    && (unsigned char)*ptr <= 0xbe)
                        *ptr -= 0x20;
                    ptr++;
                }
                string_remove(temp_line, "==");
                heading = 0x2;
            }

            pre_heading = false;
            section_line_count = -1;
        }

        /* prevent lines from breaking in the middle of words */
        ptr = temp_line;
        while (*ptr) {

            if (*ptr == ' ')
                e = INDEX_OF(ptr, temp_line);

            if (INDEX_OF(ptr, temp_line) - b >= limit) {
                if (e - b > 0)
                    temp_line[e] = '\n';
                b = e + 1;
            }

            ptr++;
        }

        string_trim(temp_line);
        line = temp_line;

        /* format and output lines */
        while (line) {
            next = strchr(line, '\n');
            if (next)
                *next = '\0';

            if (pre_heading) {
                print_line(2, "\033[3m%s\033[0m\n", line);
            } else if (heading) {
                switch (heading) {
                case 0x2:
                    print_line(0, "\033[1m%s\033[0m\n", line);
                    break;
                case 0x3:
                    print_line(1, "\033[1m%s\033[0m\n", line);
                    break;
                default:
                    print_line(2, "\033[1m\033[3m%s\033[0m\n", line);
                    break;
                }
            } else {
                print_line(2, "%s\n", line);
            }

            if (next)
                *next = '\n';
            line = next ? next + 1 : NULL;
        }

        blank_line = *temp_line == '\0';
        total_line_count++;
        section_line_count++;
    }

    free(temp_line);
}

int print_wiki(char *subject)
{
    const char  beginning[] = "\"extract\":\"";
    bool        failure     = false;
    FILE        *fptr       = NULL;
    char        *content    = NULL;
    char        *line       = NULL;
    char        *next       = NULL;
    char        *ptr        = NULL;
    long        size;

    fptr = fdopen(dup(temp_file_descriptor), "rb");
    if (fptr == NULL)
        failure = true;

    if (!failure) {
        fseek(fptr, 0, SEEK_END);
        size = ftell(fptr);
        rewind(fptr);
        content = malloc(size + 1 * (sizeof(char)));
        fread(content, sizeof(char), size, fptr);
        fclose(fptr);
    }

    if (content == NULL)
        failure = true;

    if (!failure) {
        ptr = strstr(content, beginning);
        if (ptr != NULL && *ptr)
            memmove(content,
                ptr + strlen(beginning),
                strlen(ptr) - strlen(beginning)
            );
        else
            failure = true;
    }

    if (!failure) {
        ptr = strchr(content, '}');
        if (ptr != NULL && *ptr--)
            *ptr = '\0';
        else
            failure = true;
    }

    if (!failure) {
        dictionary_replace(content, cleanup_mappings, LENGTH(cleanup_mappings));

        if ((global_options >> 0) & 1U) {
            fprintf(stdout, "%s\n", content);
        } else {
            string_remove_redundent_spaces(content);
            unicode_decode(content);

            if (!((global_options >> 1) & 1U))
                fprintf(stdout, "\033[4m%s:\033[0m\n\n", subject);

            line = content;
            while (line) {
                next = strchr(line, '\n');
                if (next)
                    *next = '\0';

                handle_line(line);

                if (next)
                    *next = '\n';
                line = next ? next + 1 : NULL;
            }
        }
    }

    if (content != NULL)
        free(content);

    return !failure;
}

int main(int argc, char *argv[])
{
    int     state       = 0;
    char    *input      = NULL;
    char    *url        = NULL;
    int     buffer_size = 80;
    char    buffer[buffer_size];
    int     input_size;
    int     h;
    char    c;

    /* validate input from STDIN */
    if (!isatty(fileno(stdin)) && fgets(buffer, buffer_size, stdin) != NULL) {
        input_size  = strlen(buffer);
        input       = (char*)malloc(input_size);
        input[0]    = '\0';
        strcat(input, buffer);

    /* validate input from CLI */
    } else if (argc >= 2) {
        input_size  = strlen(argv[argc - 1]);
        input       = (char*)malloc(input_size);
        input[0]    = '\0';
        strcat(input, argv[argc - 1]);

    /* no valid input, so show usage */
    } else {
        fprintf(stderr, "%s\n", usage);
        state = 1;
    }

    if (state == 0) {
        while ((c = getopt(argc, argv, "hl:rs:St")) != -1) {
            switch (c) {
            case 'l':
                memmove(language, optarg, 2);
                break;
            case 'r':
                global_options |= 1U << 0;
                break;
            case 's':
                h = atoi(optarg);
                if (h < 0 || h >= LENGTH(services))
                    h = 0;
                webpage = services[h].template;
                break;
            case 'S':
                for (h = 0; h < LENGTH(services); h++)
                    fprintf(stderr, "%d: %s\n", h, services[h].name);
                return 0;
            case 't':
                global_options |= 1U << 1;
                break;
            case 'h':
                fprintf(stderr, "%s\n", usage);
                fprintf(stderr, "Finds a wiki subject and prints it.\n");
                fprintf(stderr, " -h          Print this help text and exit\n");
                fprintf(stderr, " -l CODE     Language code in ISO 639-1 format\n");
                fprintf(stderr, " -r          Print wiki in raw format\n");
                fprintf(stderr, " -s ID       Specify service ID\n");
                fprintf(stderr, " -S          List services and IDs and exit\n");
                fprintf(stderr, " -t          Print wiki in text only format\n");

                free(input);
                return 0;
            default:
                fprintf(stderr, "%s\n", usage);
                state = 1;
            }
        }
    }

    /* initialization is done, so proceed with the rest of the program... */
    terminal_width = get_terminal_width();

    if (state == 0)
        url = get_wiki_url(input);

    if (state == 0 && download_wiki(url) != CURLE_OK) {
        fprintf(stderr, "%s\n", "Failed to download the wiki");
        state = 1;
    }

    if (state == 0 && !print_wiki(input)) {
        fprintf(stderr, "%s\n", "Failed to print the wiki");
        state = 1;
    }

    if (temp_file_descriptor != -1) {
        close(temp_file_descriptor);
        unlink(temp_file);
    }

    if (input != NULL)
        free(input);

    if (url != NULL)
        free(url);

    return state;
}
