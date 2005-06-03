#!/usr/bin/perl
# An example program which prints Hello World! on a page
#
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

$enc=PDFDict();
$enc->{'Type'}=PDFName('Encoding');
$enc->{'Differences'}=PDFArray(PDFNum(97), PDFName('achar'), PDFName('bchar'));
$pdf->new_obj($enc);

$a=PDFDict();
$a->{' stream'}="1000 0 0 0 750 750 d1\n0 0 m\n375 750 l\n750 0 l\nS\n20 375 m\n720 375 l\ns";
$pdf->new_obj($a);

$b=PDFDict();
$b->{' stream'}="500 0 0 0 750 750 d1\n0 0 m\n0 750 l\n120 175 575 250 75 0 c\nf";
$pdf->new_obj($b);

$chrprc=PDFDict();
$chrprc->{'achar'}=$a;
$chrprc->{'bchar'}=$b;
$pdf->new_obj($chrprc);
$fo=PDFDict();
$fo->{'Type'}=PDFName('Font');
$fo->{'Subtype'}=PDFName('Type3');
$fo->{'FontBBox'}=PDFArray(PDFNum(0), PDFNum(0), PDFNum(750), PDFNum(750));
$fo->{'FontMatrix'}=PDFArray(PDFNum(0.0001), PDFNum(0), PDFNum(0), PDFNum(0.0001), PDFNum(0), PDFNum(0));
$fo->{'CharProcs'}=$chrprc;
$fo->{'Encoding'}=$enc;
$fo->{'FirstChar'}=PDFNum(97);
$fo->{'LastChar'}=PDFNum(98);
$fo->{'Widths'}=PDFArray(PDFNum(1000), PDFNum(500));
$pdf->new_obj($fo);
$root->{'Resources'}->{'Font'}->{'FTYPE3'}=$fo;

$page->add("BT 1 0 0 1 200 800 Tm /FTYPE3 299 Tf 0 Tr (ababab) Tj ET");        # put some content on the page
$pdf->out_file($ARGV[0]);   # output the document to a file

# all done!
