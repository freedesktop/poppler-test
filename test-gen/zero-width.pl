#!/usr/bin/perl
use Text::PDF::File;
use Text::PDF::Page;        # pulls in Pages
use Text::PDF::Utils;       # not strictly needed
$pdf = Text::PDF::File->new;            # Make up a new document
$root = Text::PDF::Pages->new($pdf);    # Make a page tree in the document
$root->bbox(0, 0, 300, 300);            # hardwired page size A4 (for this app.) for all pages
$page = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree

sub draw_shape {
  my ($xscale, $yscale) = @_;
  $x = 100/$xscale;
  $y = 250/$yscale;
  $y2 = 300/$yscale;
  $page->add("$x $y m $x $y2 l " . sprintf("%d", $x + (100/$xscale)) . " $y2 l S\n"); # simple degenerate path
}

# round caps
$page->add("0 0.1 1 RG 0 w 1 J\n");
draw_shape 1, 1;

# square caps; should not draw anything
$page->add("100 0 0 1 0 -100 cm\n");
draw_shape 100, 1;

# butt caps; should not draw anything
$page->add("1 0 0 100 0 -100 cm\n");
draw_shape 100, 100;

# empty operator for fun
$page->add("s");

$pdf->out_file($ARGV[0]);   # output the document to a file
