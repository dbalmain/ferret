map <F2> :source tools/keymaps.vim<CR>
map <F12> :call <SID>FerretizeCurrent(expand("<cword>"))<CR>
map <F11> :call <SID>InternCurrent(expand("<cword>"))<CR>

function! <SID>FerretizeCurrent(word)
  normal msHmt
  let bn = bufname('%')
  if a:word =~ '^[a-z0-9_]\+$'
    let replace_str = "frt_".a:word
    echo "Replacing <".a:word."> with <".replace_str.">."
  elseif a:word =~ '^[A-Z0-9_]\+$'
    let replace_str = "FRT_".a:word
    echo "Replacing <".a:word."> with <".replace_str.">."
  elseif a:word =~ '^[A-Za-z0-9]\+$'
    let replace_str = "Frt".a:word
    echo "Replacing <".a:word."> with <".replace_str.">."
  else
    echo "No idea what to do with this one"
    normal 'tzt`s
    return
  endif
  exec(":bufdo! %s/\\<".a:word."\\>/".replace_str."/ge | update")
  exe ":silent! :b! ".bn
  normal 'tzt`s
endfunction

function! <SID>InternCurrent(word)
  exec("s/\"\\?\\<".a:word."\\>\"\\?/I(&)/")
endfunction
