#// url.rl -*-C-*-

%%{
    machine URL;

    uword = [_] | alnum;
    dword = '-' | uword;
    dalnum = '-' | alnum;
    proto = 'http'[s]? | 'ftp' | 'file';
    urlc  = alnum | [.,\/_\-\@\:];

    url =
        (
            proto  [:][/]+ %{ skip = p - ts; } dword+ ([.] uword dword*)+ |
            alnum+ [:][/]+ urlc+                                          |

            (alnum (dalnum* alnum)? [.])+  #// Subdomains
                ('com' |'edu'   | 'biz' | 'gov' |
                 'int' | 'info' | 'mil' | 'net' |
                 'org' | alpha{2})
        )

        #// Port
        ( [:] digit+ )?

        ([/]? @{ trunc = 1; });
}%%
