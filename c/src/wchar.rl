/* wchar.rl -*-C-*- */

%%{

    wdigit = '0' .. '9';

    walpha =
        (0x0340 .. 0x04cf |
         0x04e0 .. 0x09ef |
         0x0ac0 .. 0x0d6f |
         0x2000 .. 0x2a5f);

    walnum = wdigit | walpha;
}%%
