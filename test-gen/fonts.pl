#!/usr/bin/perl

use Text::PDF::File;
use Text::PDF::Page;        # pulls in Pages
use Text::PDF::Utils;       # not strictly needed
use Text::PDF::SFont;
use Text::PDF::Dict;
do "image.inc";
$pdf = Text::PDF::File->new;            # Make up a new document
$root = Text::PDF::Pages->new($pdf);    # Make a page tree in the document
$root->bbox(0, 0, 595, 840);            # hardwired page size A4 (for this app.) for all pages
$page = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree
$font = Text::PDF::SFont->new($pdf, 'Helvetica', 'F0');     # Make a new font in the document

$widths_string = "176 270 369 626 547 692 576 185 313 313 280 533 167 294 184 395 536 380 501 530 499 536 541 391 534 541 202 202 533 533 533 524 774 507 552 553 552 415 397 551 555 288 331 536 380 716 541 546 501 546 538 517 460 546 523 814 482 473 396 282 395 282 483 552 333 503 519 494 519 511 289 518 523 273 279 478 273 770 522 511 519 518 357 470 304 522 437 669 433 448 351 369 270 369 524 750 530 750 167 443 344 553 542 543 333 1024 517 197 688 750 396 750 750 167 167 344 344 347 500 1000 333 782 470 197 765 750 351 473 176 270 517 535 548 473 270 483 333 783 324 371 533 294 783 552 347 533 317 326 333 471 576 333 333 244 329 371 623 645 691 524 507 507 507 507 507 507 713 553 415 415 415 415 288 288 288 288 558 541 546 546 546 546 546 533 546 546 546 546 546 473 501 550 503 503 503 503 503 503 753 494 511 511 511 511 273 273 273 273 511 522 511 511 511 511 511 533 511 522 522 522 522 448 519 448";
@width_nums = split(/ /, $widths_string);
my @widths = ();
foreach (@width_nums) {
  	push @widths, PDFNum($_);
}

$fontdesc = PDFDict();
$pdf->new_obj($fontdesc);
$fontdesc->{'Type'} = PDFName("FontDescriptor");
$fontdesc->{'FontBBox'} = PDFArray(PDFNum(-129),PDFNum(-330),PDFNum(1261),PDFNum(1167));
$fontdesc->{'FontName'} = PDFName("NoFont");
$fontdesc->{'Flags'} = PDFNum(40);
$fontdesc->{'Ascent'} = PDFNum(1008);
$fontdesc->{'Descent'} = PDFNum(-210);
$fontdesc->{'FontWeight'} = PDFNum(900);
$fontdesc->{'CapHeight'} = PDFNum(0);
$fontdesc->{'StemV'} = PDFNum(0);
$fontdesc->{'ItalicAngle'} = PDFNum(0);

$fontdesc2 = PDFDict();
$pdf->new_obj($fontdesc2);
$fontdesc2->{'Type'} = PDFName("FontDescriptor");
$fontdesc2->{'FontBBox'} = PDFArray(PDFNum(-129),PDFNum(-330),PDFNum(1261),PDFNum(1167));
$fontdesc2->{'FontName'} = PDFName("NoFont");
$fontdesc2->{'Flags'} = PDFNum(42);
$fontdesc2->{'Ascent'} = PDFNum(1008);
$fontdesc2->{'Descent'} = PDFNum(-210);
$fontdesc2->{'CapHeight'} = PDFNum(0);
$fontdesc->{'FontWeight'} = PDFNum(900);
$fontdesc2->{'StemV'} = PDFNum(0);
$fontdesc2->{'ItalicAngle'} = PDFNum(0);

$ttfont = Text::PDF::Dict->new($pdf);
$ttfont->{'Type'} = PDFName("Font");
$ttfont->{'Subtype'} = PDFName("TrueType");
$ttfont->{'Name'} = PDFName("F1");
$ttfont->{'BaseFont'} = PDFName("NoFont");
$ttfont->{'FirstChar'} = PDFNum(32);
$ttfont->{'LastChar'} = PDFNum(255);
$ttfont->{'FontDescriptor'} = $fontdesc;
$ttfont->{'Widths'} = PDFArray(@widths);
$ttfont2 = Text::PDF::Dict->new($pdf);
$ttfont2->{'Type'} = PDFName("Font");
$ttfont2->{'Subtype'} = PDFName("TrueType");
$ttfont2->{'Name'} = PDFName("F2");
$ttfont2->{'FirstChar'} = PDFNum(32);
$ttfont2->{'LastChar'} = PDFNum(255);
$ttfont2->{'BaseFont'} = PDFName("NoFont");
$ttfont2->{'FontDescriptor'} = $fontdesc2;
$ttfont2->{'Widths'} = PDFArray(@widths);
$pdf->new_obj($ttfont);
$pdf->new_obj($ttfont2);
$root->add_font($font);                                     # Tell all pages about the font
$root->add_font($ttfont);                                     # Tell all pages about the font
$root->add_font($ttfont2);                                     # Tell all pages about the font

$x = 100;
$y = 500;
$sx = $w;
$sy = $h;

# draw some black text
$page->add("0 0 1 rg\n");
$page->add("BT 1 0 0 1 100 600 Tm /F0 48 Tf 0 Tr (Hello World!) Tj ET\n");
$page->add("BT 1 0 0 1 100 500 Tm /F1 48 Tf 0 Tr (Hello World!) Tj ET\n");
$page->add("BT 1 0 0 1 100 400 Tm /F2 48 Tf 0 Tr (Hello World!) Tj ET\n");

$page->add("q\nQ");
$pdf->out_file($ARGV[0]);   # output the document to a file

# all done!
