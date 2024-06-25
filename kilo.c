/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


/*** data ***/

struct termios orig_termios;

/*** terminal ***/

// error handling.
void die(const char *s) {
	perror(s);
	exit(1);
}

// restore the terminal to the original state when the program exits.
void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {// TCSAFLUSH specifies when to apply change to the terminal - waits for all pending output and discard input not read.
		die("tcsetattr");
	}
}

// switching the terminal from canonical/cooked mode to raw mode.
// turn off echoing (echoing - input appearing as they are typed).
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    	die("tcgetattr");
	}
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP| IXON); // ~IXON turns off Ctrl-S & Ctrl-Q, ICRNL fixes Ctrl-M.
	raw.c_oflag &= (OPOST); // turns off output processing ("\r\n").
	raw.c_cflag &= (CS8); // sets character size to 8 bits/byte.
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); // ~ICANON turns off canonical mode and allow reading byte-by-byte instead of line-by-line. ISIG turns off Ctrl-C and Ctrl-Z signals.
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1; // timeout for read().

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
		die("tcsetattr");
	}
}

/*** init ***/

int main () {
	enableRawMode();

	char c;
	while (1) {
		char c = '\0';
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
			die("read");
		}
    	if (iscntrl(c)) {
    		printf("%d\r\n", c);
    	} else {
    		printf("%d ('%c')\r\n", c, c);
    	}
		if (c == 'q') break;
    }

    return 0;
}