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
$root->{'Resources'}->{'XObject'}=PDFDict();
$count = 0;
$column = 0;
foreach $file (glob "jpegs/*.jpg") {
	my ($xo,$key,$w,$h)=getImageObjectFromFile($file);
	my $key = $key."$column";

	$pdf->new_obj($xo);
	$root->{'Resources'}->{'XObject'}->{$key}=$xo;

	$page->add("q\n"); #saveState
	$x = 100;
	$y = 100;
	$sx = $w/3;
	$sy = $h/3;
	$page->add(sprintf("%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f cm\n", $sx,0,0,$sy,$x,$y+$count*$sy));
	$page->add("/$key Do\n");
	$page->add("Q\n"); #restoreState
	$count += 1;
}
$column++;
$count = 0;
foreach $file (glob "jpegs/*.jpg") {
	my ($xo,$key,$w,$h)=getImageObjectFromFile($file);
	my $key = $key."$column";
        $params = PDFDict();
	$params->{'ColorTransform'}=PDFNum(1);
	$xo->{'DecodeParms'}=PDFArray($params);
	$pdf->new_obj($xo);
	# we need to use an array because /Filter is an array
	$root->{'Resources'}->{'XObject'}->{$key}=$xo;

	$page->add("q\n"); #saveState
	$x = 100;
	$y = 100;
	$sx = $w/3;
	$sy = $h/3;
	$page->add(sprintf("%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f cm\n", $sx,0,0,$sy,$x+$sx*2,$y+$count*$sy));
	$page->add("/$key Do\n");
	$page->add("Q\n"); #restoreState
	$count += 1;
}
$column++;
$count = 0;
foreach $file (glob "jpegs/*.jpg") {
	my ($xo,$key,$w,$h)=getImageObjectFromFile($file);
	my $key = $key."$column";
	#$params = PDFArray(PDFName('ColorTransform'), PDFNum(0));
        $params = PDFDict();
	$params->{'ColorTransform'}=PDFNum(0);
	# we need to use an array because /Filter is an array
	$xo->{'DecodeParms'}=PDFArray($params);
	$pdf->new_obj($xo);
	$root->{'Resources'}->{'XObject'}->{$key}=$xo;

	$page->add("q\n"); #saveState
	$x = 100;
	$y = 100;
	$sx = $w/3;
	$sy = $h/3;
	$page->add(sprintf("%0.3f %0.3f %0.3f %0.3f %0.3f %0.3f cm\n", $sx,0,0,$sy,$x+$sx*4,$y+$count*$sy));
	$page->add("/$key Do\n");
	$page->add("Q\n"); #restoreState
	$count += 1;
}

$page->add("q\n");
$page->add("Q");
$pdf->out_file($ARGV[0]);   # output the document to a file

# all done!
