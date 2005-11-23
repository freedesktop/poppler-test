#!/usr/bin/perl

use Text::PDF::File;
use Text::PDF::Page;        # pulls in Pages
use Text::PDF::Utils;       # not strictly needed
use Text::PDF::SFont;
do "image.inc";
$pdf = Text::PDF::File->new;            # Make up a new document
$root = Text::PDF::Pages->new($pdf);    # Make a page tree in the document
$root->proc_set("PDF", "Text");         # Say that all pages have PDF and Text instructions
$root->bbox(0, 0, 595, 840);            # hardwired page size A4 (for this app.) for all pages
$page = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree
$font = Text::PDF::SFont->new($pdf, 'Helvetica', 'F0');     # Make a new font in the document
$root->add_font($font);                                     # Tell all pages about the font

#my ($w,$h,$bpc,$cs,$img)=parseImage('inline-image.ppm');
my ($w,$h,$bpc,$cs,$img)=parseImage('romedalen.ppm');
my $key='IMG1';
$page->add("q\n"); #saveState
$x = 100;
$y = 500;
$sx = $w/3;
$sy = $h/3;
#$page->add("0 0 34 rg\n");
$page->add(sprintf("%0.6f %0.6f %0.6f %0.6f %0.6f %0.6f cm\n", $sx,0,0,$sy,$x+100,$y));
$page->add("BI\n");
$page->add("/W $w\n");
$page->add("/H $h\n");
$page->add("/CS /RGB\n");
$page->add("/BPC $bpc\n");
$page->add("ID\n");
$page->add($img);
$page->add("\nEI\n");
$page->add("Q\n"); #restoreState
$page->add("q\n");
$page->add("0 0 34 rg\n");
$page->add(sprintf("%0.6f %0.6f %0.6f %0.6f %0.6f %0.6f cm\n", $sx,0,0,$sy,$x,$y));
$page->add("BI\n");
$page->add("/W $w\n");
$page->add("/H $h\n");
$page->add("/CS /RGB\n");
$page->add("/BPC $bpc\n");
$page->add("ID\n");
$page->add($img);
$page->add("\nEI\n");

$page->add("Q"); #restoreState
$pdf->out_file($ARGV[0]);   # output the document to a file

# all done!
