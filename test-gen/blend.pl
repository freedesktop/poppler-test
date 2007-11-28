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

my ($w,$h,$bpc,$cs,$img)=parseImage('romedalen.ppm');
my $key='IMG1';
$xo=PDFDict();
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
$root->{'Resources'}->{'ExtGState'}=PDFDict();

@modes=('Normal', 'Multiply', 'Screen', 'Overlay', 'Darken', 'Lighten', 'ColorDodge', 'ColorBurn', 'HardLight',
	'SoftLight', 'Difference', 'Exclusion','Hue','Saturation','Color','Luminocity' );
$state_count = 0;
@gstates=();
foreach $mode (@modes) {
	$td=PDFDict();
	$td->{'ca'}=PDFNum(0.7);
	$td->{'CA'}=PDFNum(1.0);
	$td->{'BM'}=PDFName($mode);
	$td->{'Type'}=PDFName('ExtGState');
	$td->{'OPM'}=PDFNum(1);
	$td->{'AIS'}=PDFBool('false');
	$td->{'SMask'}=PDFName('None');
	$pdf->new_obj($td);
	$gstate = "RE$state_count";
	$state_count += 1;
	push @gstates, $gstate;

	$root->{'Resources'}->{'ExtGState'}->{$gstate}=$td;
}
$page->add("q\n"); #saveState
$page->add("0.51373 0.54510 0.54510 rg\n");
$page->add("0.0 0.4 0.4 RG\n");
$page->add("40.0 w\n");
$y = 150.0;
for ($i=0; $i<3; $i++) {
	$page->add("$y 0.0 m\n");
	$page->add("$y 1000 l\n");
	$y += 100.0;
	$page->add("S\n");
}

$x = 100;
$y = 100;
$sx = $w/3;
$sy = $h/3;

foreach $gstate (@gstates) {
	$page->add("q\n"); #saveState
	$page->add("/$gstate gs\n");
	$page->add(sprintf("%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f cm\n", $sx,0,0,$sy,$x,$y));
	$page->add("/$key Do\n");
	$y += 100;
	if ($y > 700) {
		$x += 100;
		$y = 100;
	}
	$page->add("Q\n"); #restoreState
}
$page->add("Q"); #restoreState
$pdf->out_file($ARGV[0]);   # output the document to a file

# all done!
