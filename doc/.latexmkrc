#!/usr/bin/env perl
# latexmk 設定ファイル
# uplatex + dvipdfmx によるビルドチェーン

$latex = 'uplatex -synctex=1 -interaction=nonstopmode -file-line-error %O %S';
$dvipdf = 'dvipdfmx %O -o %D %S';
$pdf_mode = 3;
$bibtex = 'pbibtex %O %B';
$makeindex = 'mendex %O -o %D %S';
$jobname = 'モジュール設計仕様書';
