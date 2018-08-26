wikid
=====
wikid is a quick and simple CLI-program for downloading and rendering wikipedia
pages in your terminal. And it's not **wikid** as in *wiki daemon*, but as in
*wicked, Strikingly good, and effective*.

Requirements
------------
wikid uses the **curl** library to download the wiki pages, and **ncurses** for
retrieving the terminal width for line output.

Installation
------------
Edit config.mk to match your local setup (wikid is installed into the
/usr/local namespace by default), then simply enter the following command to
install (if necessary as root):

    make clean install

Example usage of wikid
----------------------
Read about wikipedia, using `less`

    wikid wikipedia | less

Read about wikipedia in german, using `less`

    wikid -lde wikipedia | less
