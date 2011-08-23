" original file is clang_complete.vim

" File: az_complete.vim
" Author: Constellation <utatane.tea@gmail.com>
"
" Description: in order to complete in JavaScript.
"

let s:source = {
      \ 'name': 'az_complete',
      \ 'kind': 'ftplugin',
      \ 'filetypes': { 'javascript': 1 },
      \ }

function! s:source.initialize() "{{{
  if !exists('g:az_complete_auto')
    let g:az_complete_auto = 0
  endif
endfunction "}}}

function! s:source.finalize() "{{{
endfunction "}}}

function! s:source.get_keyword_pos(cur_text) "{{{
  if neocomplcache#within_comment()
    return -1
  endif
  let l:line = getline('.')
  let l:start = col('.') - 1
  while l:start > 0 && l:line[l:start - 1] =~ '\i'
    let l:start -= 1
  endwhile
  return l:start
endfunction "}}}

function! s:source.get_complete_words(cur_keyword_pos, cur_keyword_str) "{{{
  if neocomplcache#is_auto_complete() && g:az_complete_auto == 0
    return []
  endif

  let l:buf = getline(1, '$')
  let l:tempfile = expand('%:p:h') . '/' . localtime() . expand('%:t')

  try
    call writefile(l:buf, l:tempfile)
    let l:escaped_tempfile = shellescape(l:tempfile)
    let l:offset = line2byte('.') + a:cur_keyword_pos - 1
    let l:command = 'az ' . l:escaped_tempfile . ' --pulse=' . l:offset
    let l:output = split(neocomplcache#system(l:command), "\n")
  finally
    call delete(l:tempfile)
  endtry

  if v:shell_error
    return []
  endif

  let l:list = []
  for l:line in l:output
    let l:idx = stridx(l:line, "#")
    let l:target = l:line[:l:idx - 1]
    let l:proto = l:line[l:idx + 1:]
    let l:item = {
                \ "word": l:target,
                \ "menu": '[az] '. l:proto,
                \ "dup": 1,
                \ "rank": 10000,
                \ }
    call add(l:list, l:item)
  endfor
  return neocomplcache#keyword_filter(l:list, a:cur_keyword_str)
endfunction "}}}

function! neocomplcache#sources#az_complete#define() "{{{
  return s:source
endfunction "}}}
