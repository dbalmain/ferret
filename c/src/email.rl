#// email.rl -*-C-*-
%%{
    machine Email;

    #// RFC 2822 - matching email addresses
    NO_WS_CTL       = ( 1..8 | 11 | 12 | 14..31 | 127 );
    ASCII           = 1..127;
    atext           = [a-zA-Z0-9!#$%&\'*+\-/=?^_`{|}~];
    qtext           = ( NO_WS_CTL | 33 | 35..91 | 93..126 );
    dtext           = ( NO_WS_CTL | 33..90 | 94..126 );
    dot_atom        = atext+ ('.' atext+)*;
    text            = ( 1..9 | 11 | 12 | 14..127 );
    quoted_pair     = '\\' text;
    quoted_string   = '"' ( qtext | quoted_pair )* '"';
    domain_literal  = '[' (dtext | quoted_pair)* ']';

    local_part      = dot_atom | quoted_string;
    domain          = dot_atom | domain_literal;

    email           = local_part '@' domain;
}%%
