#!/usr/bin/perl
# An example program which prints Hello World! on a page
#
use Text::PDF::File;
use Text::PDF::Page;        # pulls in Pages
use Text::PDF::Utils;       # not strictly needed
$pdf = Text::PDF::File->new;            # Make up a new document
$root = Text::PDF::Pages->new($pdf);    # Make a page tree in the document
$root->bbox(0, 0, 300, 300);            # hardwired page size A4 (for this app.) for all pages
$page = Text::PDF::Page->new($pdf, $root);      # Make a new page in the tree

sub draw_dots {
  $page->add("100 250 m 100 250 l S\n"); # simple degenerate path
  $page->add("150 250 m s\n"); # simple closed degenerate path
  $page->add("200 250 m h S\n"); # explicit closed degenerate path
  $page->add("q\n");
  $page->add("[10 15] 0 d\n");
  $page->add("220 250 m 220 250 l s\n");
  $page->add("Q\n");
}

# round caps
$page->add("0 0.1 1 RG 13 w 1 J\n");
draw_dots;

# square caps; should not draw anything
$page->add("1 0 0 1 0 -100 cm\n");
$page->add("2 J\n");
draw_dots;

# butt caps; should not draw anything
$page->add("1 0 0 1 0 -100 cm\n");
$page->add("0 J\n");
draw_dots;

# empty operator for fun
$page->add("s");

$pdf->out_file($ARGV[0]);   # output the document to a file
