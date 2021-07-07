# etui_md

test program for markdown renderer using the EFL.

To be integrated later in Etui.

compilation :

``
gcc -g -Wall -Wextra -o etui_md_test *.c `pkg-config --cflags --libs elementary` -I.
``

test with one of markdown files (.txt exension in the current dir). For example :

./etui_md_test underline.txt
