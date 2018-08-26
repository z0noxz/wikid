/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <term.h>

unsigned get_terminal_width(void)
{
    const char* term = getenv("TERM");
    char buff[1024];
    int cols;

    if (!term)
        return 0;

    if (tgetent(buff, term) <= 0)
        return 0;

    cols = tgetnum("co");

    if (cols == -1)
        return 0;

    return cols;
}
