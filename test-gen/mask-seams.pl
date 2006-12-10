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

my ($w,$h,$bpc,$cs,$img)=parseImage('solid.pbm');
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
my $scale=4.0;
my $hscale=16.0;
my $spacing=2.0;
$page->add("q\n");
for (my $i=0; $i<16; $i++) {
  $page->add("q\n"); #saveState
  $page->add("0 0 1 rg\n");
  $page->add(sprintf("%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f cm\n", $w/$scale,0,0,$h/$hscale,100,600-$i*($h/$hscale+$spacing)));
  $page->add("/$key Do\n");
  $page->add("Q\n"); #restoreState
  $page->add("q\n"); #saveState
  $page->add("0 0 1 rg\n");
  $page->add(sprintf("%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f cm\n", $w/$scale,0,0,$h/$hscale,100+$w/$scale+0.1*$i,600-$i*($h/$hscale+$spacing)));
  $page->add("/$key Do\n");
  $page->add("Q\n"); #restoreState
}
$page->add("Q");
$pdf->out_file($ARGV[0]);   # output the document to a file

