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
$page2 = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree
$page3 = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree
$page4 = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree
$page5 = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree
$page6 = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree

$page->bbox(0, 0, 800, 100);
$page->{'CropBox'}=PDFArray(PDFNum(0), PDFNum(0), PDFNum(1000), PDFNum(1000));
$page2->bbox(0, 0, 300, 860);
$page2->{'CropBox'}=PDFArray(PDFNum(0), PDFNum(0), PDFNum(1000), PDFNum(200));
$page3->bbox(0, 0, 860, 300);
$page3->{'CropBox'}=PDFArray(PDFNum(0), PDFNum(0), PDFNum(200), PDFNum(1000));
$page4->bbox(0, 0, 1000, 1000);
$page4->{'CropBox'}=PDFArray(PDFNum(0), PDFNum(0), PDFNum(800), PDFNum(200));
$page5->bbox(0, 0, 595, 841);
$page5->{'CropBox'}=PDFArray(PDFNum(0), PDFNum(0), PDFNum(595), PDFNum(841));
$page6->bbox(0, 0, 578, 1224);
$page6->{'CropBox'}=PDFArray(PDFNum(0), PDFNum(421), PDFNum(579), PDFNum(1224));
$pdf->out_file($ARGV[0]);   # output the document to a file


# all done!
