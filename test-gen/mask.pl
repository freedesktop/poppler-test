#!/usr/bin/perl -w

use strict;
use Text::PDF::File;
use Text::PDF::Page;        # pulls in Pages
use Text::PDF::Utils;       # not strictly needed
use Text::PDF::SFont;
do "image.inc";
my $pdf = Text::PDF::File->new;            # Make up a new document
my $root = Text::PDF::Pages->new($pdf);    # Make a page tree in the document
$root->proc_set("PDF", "Text");         # Say that all pages have PDF and Text instructions
$root->bbox(0, 0, 595, 840);            # hardwired page size A4 (for this app.) for all pages
my $page = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree

my ($w,$h,$bpc,$cs,$img)=parseImage('a.pbm');
my $key='IMG1';
my $xo=PDFDict();
$xo->{'ImageMask'}=PDFBool('true');
$xo->{'Type'}=PDFName('XObject');
$xo->{'Subtype'}=PDFName('Image');
$xo->{'Name'}=PDFName($key);
$xo->{'Width'}=PDFNum($w);
$xo->{'Height'}=PDFNum($h);
$xo->{'Filter'}=PDFArray(PDFName('FlateDecode'));
$xo->{'BitsPerComponent'}=PDFNum($bpc);
$xo->{'ColorSpace'}=PDFName($cs);
$xo->{' stream'}=$img;
$pdf->new_obj($xo);
$root->{'Resources'}->{'XObject'}=PDFDict();
$root->{'Resources'}->{'XObject'}->{$key}=$xo;

$page->add("q\n"); #saveState
$page->add("0 0 34 rg\n");
$page->add(sprintf("%0.6f %0.6f %0.6f %0.6f %0.6f %0.6f cm\n", $w,0,0,$h,100,500));
$page->add("/$key Do\n");
$page->add("Q"); #restoreState
$pdf->out_file($ARGV[0]);   # output the document to a file

