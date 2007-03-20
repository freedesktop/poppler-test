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
$root->bbox(0, 0, 595, 840);            # hardwired page size A4 (for this app.) for all pages
$page = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree
$font = Text::PDF::SFont->new($pdf, 'Helvetica', 'F0');     # Make a new font in the document
$root->add_font($font);                                     # Tell all pages about the font

$char_commands = "1000 0 0 0 750 750 d1\n0 0 m\n375 750 l\n750 0 l\nS\n20 375 m\n720 375 l\ns";
$chrprc=PDFDict();

@differences = (PDFNum(97));
$char_count = 0;
sub build_char {
  my $name = shift;
  $tmp = PDFDict();
  $char_count = $char_count+1;
  $tmp->{' stream'}=$char_commands;
  $pdf->new_obj($tmp);
  $chrprc->{$name}=$tmp;
  push @differences, PDFName($name);
}

build_char('bullet');
build_char('ff');
build_char('V');
build_char('Ycircumflex'); #acroreader treats as plain Y

#non standard
build_char('uni2022');
build_char('f_i');
build_char('T_h');
build_char('l_quotesingle');
build_char('afii57414');
build_char('P.swash');
build_char('s.sc');

$pdf->new_obj($chrprc);

$enc=PDFDict();
$enc->{'Type'}=PDFName('Encoding');
$enc->{'Differences'}=PDFArray(@differences);
$pdf->new_obj($enc);

$fo=PDFDict();
$fo->{'Type'}=PDFName('Font');
$fo->{'Subtype'}=PDFName('Type3');
$fo->{'FontBBox'}=PDFArray(PDFNum(0), PDFNum(0), PDFNum(750), PDFNum(750));
$fo->{'FontMatrix'}=PDFArray(PDFNum(0.0001), PDFNum(0), PDFNum(0), PDFNum(0.0001), PDFNum(0), PDFNum(0));
$fo->{'CharProcs'}=$chrprc;
$fo->{'Encoding'}=$enc;
$fo->{'FirstChar'}=PDFNum(97);
$fo->{'LastChar'}=PDFNum(97+$char_count);
my @widths = ();
$string = "";
for ($i = 0; $i < $char_count; $i++) {
  push @widths, PDFNum(700);
  $string = $string . chr($i + 97);
}
#$fo->{'Widths'}=PDFArray(PDFNum(1000), PDFNum(1000), PDFNum(1000), PDFNum(1000), PDFNum(1000), PDFNum(1000), PDFNum(1000), PDFNum(1000));
$fo->{'Widths'}=PDFArray(@widths);
$pdf->new_obj($fo);
$root->{'Resources'}->{'Font'}->{'FTYPE3'}=$fo;

$page->add("BT 1 0 0 1 200 800 Tm /FTYPE3 299 Tf 0 Tr ($string) Tj ET");        # put some content on the page
$pdf->out_file($ARGV[0]);   # output the document to a file

# all done!
