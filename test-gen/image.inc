# taken from text-pdf-api-0.701.4 
# it is copyrighted by Alfred Reibenschuh
# and can be used under Perl's "Artistic License"

sub parseImage {
	my $file=shift @_;
	my $type=shift @_;
	my @css=qw( DeviceNone DeviceGray DeviceNone DeviceRGB DeviceCMYK );
	my ($buf);
	my ($w,$h,$bpc,$cs,$img)=(0,0,'','','');
	if(!$type){
		open(INF,$file);
		read(INF,$buf,100,0);
		close(INF);
		if($buf=~/^GIF8[7,9]a/) {
			$type='GIF';
		} elsif ($buf=~/^\xFF\xD8/) {
			$type='JPEG';
		} elsif ($buf=~/^\x89PNG/) {
			$type='PNG';
		} elsif ($buf=~/^P[456][\s\n]/) {
			$type='PPM';
		} elsif ($buf=~/^BM/) {
			$type='BMP';
		}
	}

	if($type eq 'PNG') {
		($w,$h,$bpc,$cs,$img)=parsePNG($file,$buf);
	} elsif($type eq 'PPM') {
		($w,$h,$bpc,$cs,$img)=parsePNM($file,$buf);
	} else {
		die "unsupported image type='$type' ";
	#	eval(qq| use Text::PDF::API::$type; |);
	#	($w,$h,$cs,$img)= eval(' return (Text::PDF::API::'.$type.'::read'.$type.'("'.$file.'")); ');
	#	$bpc=8;
	#	$cs=$css[$cs]; 
	}
	return ($w,$h,$bpc,$cs,$img);
}

sub parsePNM {
	my $file=shift @_;
	my $buf=shift @_;
	my ($t,$s,$line);
	my ($w,$h,$bpc,$cs,$img,@img)=(0,0,'','','');
	open(INF,$file);
	binmode(INF);
	$buf=<INF>;
	$buf.=<INF>;
	($t)=($buf=~/^(P\d+)\s+/);
	if($t eq 'P4') {
		($t,$w,$h)=($buf=~/^(P\d+)\s+(\d+)\s+(\d+)\s+/);
		$bpc=1;
		$s=0;
		for($line=(($w+7)*$h/8);$line>0;$line--) {
			read(INF,$buf,1);
			push(@img,$buf);
		}
		$cs='DeviceGray';
	} elsif($t eq 'P5') {
		$buf.=<INF>;
		($t,$w,$h,$bpc)=($buf=~/^(P\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+/);
		if($bpc==255){
			$s=0;
		} else {
			$s=255/$bpc;
		}
		$bpc=8;
		for($line=($w*$h);$line>0;$line--) {
			read(INF,$buf,1);
			if($s>0) {
				$buf=pack('C',(unpack('C',$buf)*$s));
			}
			push(@img,$buf);
		}
		$cs='DeviceGray';
	} elsif($t eq 'P6') {
		$buf.=<INF>;
		($t,$w,$h,$bpc)=($buf=~/^(P\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+/);
		if($bpc==255){
			$s=0;
		} else {
			$s=255/$bpc;
		}
		$bpc=8;
		if($s>0) {
			for($line=($w*$h);$line>0;$line--) {
				read(INF,$buf,1);
				push(@img,pack('C',(unpack('C',$buf)*$s)));
				read(INF,$buf,1);
				push(@img,pack('C',(unpack('C',$buf)*$s)));
				read(INF,$buf,1);
				push(@img,pack('C',(unpack('C',$buf)*$s)));
			}
		} else {
			@img=<INF>;
		}
		$cs='DeviceRGB';
	}
	close(INF);
	return ($w,$h,$bpc,$cs,join('',@img));
}

sub parsePNG {
	my $file=shift @_;
	my $buf=shift @_;
	my ($l,$crc,$w,$h,$bpc,$cs,$cm,$fm,$im,@pal,$img,@img,$filter);
	open(INF,$file);
	binmode(INF);
	seek(INF,8,0);
	while(!eof(INF)) {
		read(INF,$buf,4);
		$l=unpack('N',$buf);
		read(INF,$buf,4);
		if($buf eq 'IHDR') {
			read(INF,$buf,$l);
			($w,$h,$bpc,$cs,$cm,$fm,$im)=unpack('NNCCCCC',$buf);
			if($im>0) {die "PNG InterlaceMethod=$im not supported";}
		} elsif($buf eq 'PLTE') {
			while($l) {
				read(INF,$buf,3);
				push(@pal,$buf);
				$l-=3;
			}
		} elsif($buf eq 'IDAT') {
			while($l>512) {
				read(INF,$buf,512);
				push(@img,$buf);
				$l-=512;
			}
			read(INF,$buf,$l);
			push(@img,$buf);
		} elsif($buf eq '') {
		} elsif($buf eq 'IEND') {
			last;
		} else {
			# skip ahead
			seek(INF,$l,1);
		}
		read(INF,$buf,4);
		$crc=$buf;
	}
	close(INF);
	$img=join('',@img);
	use Compress::Zlib;
	$img=uncompress($img);
	@img=split(//,$img);
	$img='';
	my $bpcm=($bpc>8) ? 8 : $bpc/8;
	foreach my $y (1..$h) {
		$filter=unpack('C',shift(@img));
		if($filter>0){
			##die "PNG FilterType=$filter unsupported";
		}
		foreach my $x (1..POSIX::ceil($w*$bpcm)) {
			if($cs==0) { # grayscale
				if($bpc==1) {
					$buf=shift(@img);
					$buf=unpack('C',$buf);
					foreach my $bit (7,6,5,4,3,2,1,0) {
						$img.=pack('C',(($buf >> $bit) & 1)*255);
					}
				} elsif($bpc==2) {
					$buf=shift(@img);
					$buf=unpack('C',$buf);
					foreach my $bit (6,4,2,0) {
						$img.=pack('C',((($buf >> $bit) & 3)+1)*64-1);
					}
				} elsif($bpc==4) {
					$buf=shift(@img);
					$buf=unpack('C',$buf);
					foreach my $bit (4,0) {
						$img.=pack('C',((($buf >> $bit) & 15)+1)*16-1);
					}
				} elsif($bpc==8) {
					$img.=shift(@img);
				} elsif($bpc==16) {
					$buf=shift(@img).shift(@img);
					$buf=unpack('n',$buf);
					$buf=(($buf+1)/256)-1;
					$img.=pack('C',$buf);
				}
			} elsif($cs==2) { # RGB
				if($bpc==8) {
					$img.=shift(@img).shift(@img).shift(@img);
				} elsif($bpc==16) {
					foreach(1..3) {
						$buf=shift(@img).shift(@img);
						$buf=unpack('n',$buf);
						$buf=(($buf+1)/256)-1;
						$img.=pack('C',$buf);
					}
				}
			} elsif($cs==3) { # indexed
				if($bpc==1) {
					$buf=shift(@img);
					$buf=unpack('C',$buf);
					foreach my $bit (7,6,5,4,3,2,1,0) {
						$img.=$pal[(($buf >> $bit) & 1)];
					}
				} elsif($bpc==2) {
					$buf=shift(@img);
					$buf=unpack('C',$buf);
					foreach my $bit (6,4,2,0) {
						$img.=$pal[(($buf >> $bit) & 3)];
					}
				} elsif($bpc==4) {
					$buf=shift(@img);
					$buf=unpack('C',$buf);
					foreach my $bit (4,0) {
						$img.=$pal[(($buf >> $bit) & 15)];
					}
				} elsif($bpc==8) {
					$img.=$pal[unpack('C',shift(@img))];
				}
			} elsif($cs==4) { # gray + alpha
				if($bpc==8) {
					$img.=shift(@img);
					shift(@img);
				} elsif($bpc==16) {
					$buf=shift(@img).shift(@img);
					$buf=unpack('n',$buf);
					$buf=(($buf+1)/256)-1;
					$img.=pack('C',$buf);
					shift(@img);
					shift(@img);
				}
			} elsif($cs==6) { # RGB + alpha
				if($bpc==8) {
					$img.=shift(@img).shift(@img).shift(@img);
					shift(@img);
				} elsif($bpc==16) {
					foreach(1..3) {
						$buf=shift(@img).shift(@img);
						$buf=unpack('n',$buf);
						$buf=(($buf+1)/256)-1;
						$img.=pack('C',$buf);
					}
					shift(@img);
					shift(@img);
				}
			}
		}
	}
	if( ($cs==0) || ($cs==4) ) { 
		$cs='DeviceGray';
	} elsif ( ($cs==2) || ($cs==3) || ($cs==6) ) {
		$cs='DeviceRGB';
	} else {
		$cs='';
	}
	$bpc=8; # all images have been converted to 8bit values !!
	return ($w,$h,$bpc,$cs,$img);
}

sub getImageObjectFromRawData {
	my ($key,$w,$h,$bpc,$cs,$img)=@_;
	my ($xo);
	
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
	return($xo,$key,$w,$h);
}

sub getImageObjectFromPPMFile {
	my $file=shift @_;
	my ($w,$h,$bpc,$cs,$img)=parsePNM($file);
	my ($xo);
	my $key=uc($file);
	$key=~s/[^a-z0-9]+//gi;
	$key='IMGxPPMx'.uc($key).'x'.$w.'x'.$h.'x'.$cs.'x'.$bpc;
	return(getImageObjectFromRawData($key,$w,$h,$bpc,$cs,$img));
}

sub getImageObjectFromPNGFile {
	my $file=shift @_;
	my ($w,$h,$bpc,$cs,$img)=parsePNG($file);
	my ($xo);
	my $key=uc($file);
	$key=~s/[^a-z0-9]+//gi;
	$key='IMGxPNGx'.uc($key).'x'.$w.'x'.$h.'x'.$cs.'x'.$bpc;
	return(getImageObjectFromRawData($key,$w,$h,$bpc,$cs,$img));
}

sub getImageObjectFromJPEGFile {
	my $file=shift @_;
	my ($buf, $p, $h, $w, $c,$xo);
	use Text::PDF::Utils;
	my $key=uc($file);
	$key=~s/[^a-z0-9]+//gi;

	open(JF,$file);
	binmode(JF);
	read(JF,$buf,2);

	while (1) {
		read(JF,$buf,4);
		my($ff, $mark, $len) = unpack("CCn", $buf);
		last if( $ff != 0xFF);
		last if( $mark == 0xDA || $mark == 0xD9);  # SOS/EOI
		last if( $len < 2);
		last if( eof(JF));
		read(JF,$buf,$len-2);
		next if ($mark == 0xFE);
		next if ($mark >= 0xE0 && $mark <= 0xEF);
		if (($mark >= 0xC0) && ($mark <= 0xCF)) {
			($p, $h, $w, $c) = unpack("CnnC", substr($buf, 0, 6));
			last;
		}
	}
	close(JF);
	
	$key='IMGxJPGx'.uc($key).'x'.$w.'x'.$h.'x'.$c.'x'.$p;

	$xo=PDFDict();
	$xo->{'Type'}=PDFName('XObject');
        $xo->{'Subtype'}=PDFName('Image');
        $xo->{'Name'}=PDFName($key);
        $xo->{'Width'}=PDFNum($w);
        $xo->{'Height'}=PDFNum($h);
        $xo->{'Filter'}=PDFArray(PDFName('DCTDecode'));
        $xo->{' nofilt'}=1;
        $xo->{'BitsPerComponent'}=PDFNum($p);
        if($c==3) {
                $xo->{'ColorSpace'}=PDFName('DeviceRGB');
        } elsif($c==4) {
                $xo->{'ColorSpace'}=PDFName('DeviceCMYK');
        } elsif($c==1) {
                $xo->{'ColorSpace'}=PDFName('DeviceGray');
        }

        $xo->{' streamfile'}=$file;
	return($xo,$key,$w,$h);
}
	
sub getImageObjectFromFile {
        my $file=shift @_;
        my $type=shift @_;
        my ($buf);
        if(!$type){
                open(INF,$file);
                read(INF,$buf,100,0);
                close(INF);
                if($buf=~/^GIF8[7,9]a/) {
                        $type='GIF';
                } elsif ($buf=~/^\xFF\xD8/) {
                        $type='JPEG';
                } elsif ($buf=~/^\x89PNG/) {
                        $type='PNG';
                } elsif ($buf=~/^P[456][\s\n]/) {
                        $type='PPM';
		} elsif ($buf=~/^BM/) {
			$type='BMP';
                }
        }
	close(INF);
	if($type eq 'JPEG') {
		return(getImageObjectFromJPEGFile($file));
	} elsif($type eq 'PNG') {
		return(getImageObjectFromPNGFile($file));
	} elsif($type eq 'PPM') {
		return(getImageObjectFromPPMFile($file));
	} elsif($type eq 'GIF') {
		eval q/ use Text::PDF::API::GIF; /;
		my @css=qw( DeviceNone DeviceGray DeviceNone DeviceRGB DeviceCMYK );
		my ($w,$h,$cs,$img)=Text::PDF::API::GIF::readGIF($file);
		my $bpc=8;
		$cs=$css[$cs]; 
		my $key=uc($file);
		$key=~s/[^a-z0-9]+//gi;
		$key='IMGxPluginx'.$type.'x'.uc($key).'x'.$w.'x'.$h.'x'.$cs.'x'.$bpc;
		return(getImageObjectFromRawData($key,$w,$h,$bpc,$cs,$img));
	} else {
		die "unsupported image type='$type' ";
	#	eval(qq| use Text::PDF::API::$type; |);
	#	if($@) {
	#		print STDERR "imageformat '$type' unsupported.\n";
	#		return (undef);
	#	} else {
	#		my @css=qw( DeviceNone DeviceGray DeviceNone DeviceRGB DeviceCMYK );
	#		my ($w,$h,$cs,$img)= eval(' return (Text::PDF::API::'.$type.'::read'.$type.'("'.$file.'")); ');
	#		my $bpc=8;
	#		$cs=$css[$cs]; 
	#		my $key=uc($file);
	#		$key=~s/[^a-z0-9]+//cgi;
	#		$key='IMGxPluginx'.$type.'x'.uc($key).'x'.$w.'x'.$h.'x'.$cs.'x'.$bpc;
	#		return(getImageObjectFromRawData($key,$w,$h,$bpc,$cs,$img));
	#	}
	}
}

1;

__END__
