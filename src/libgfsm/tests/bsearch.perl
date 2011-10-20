#!/usr/bin/perl -w

*bsearch = \&bsearch_1;
sub bsearch_1 {
  my ($key,$l) = @_;
  my $min = 0;
  my $max = scalar(@$l);
  my ($mid,$cmp);
  while ($min < $max) {
    $mid = int($min + ($max-$min)/2);
    $cmp = ($l->[$mid] <=> $key);
    print "> CHECK(min=$min, max=$max, mid=$mid): , cmp = (l[mid=$mid]=$l->[$mid] <=> key=$key) = $cmp\n";
    if ($cmp < 0) { $min=$mid+1; }
    else          { $max=$mid; }
  }
  print "> RETURN(min=$min, max=$max; mid=$mid, cmp=$cmp)\n";
  return $min;
}

#*bsearch = \&bsearch_2;
sub bsearch_2 {
  my ($key,$l) = @_;
  my $min = 0;
  my $max = $#$l;
  my ($mid,$cmp);
  while ($min <= $max) {
    $mid = int($min + ($max-$min)/2);
    $cmp = ($l->[$mid] <=> $key);
    print "> CHECK(min=$min, max=$max, mid=$mid): , cmp = (l[mid=$mid]=$l->[$mid] <=> key=$key) = $cmp\n";
    if ($cmp < 0) { $min=$mid+1; }
    else          { $max=$mid-1; }
  }
  print "> RETURN(min=$min, max=$max; mid=$mid, cmp=$cmp)\n";
  return $min;
}



my ($key,@l) = @ARGV;
my $lstr = join(', ',map {"$_:$l[$_]"} (0..$#l));
print "bsearch(key=$key, l=[$lstr])\n";
my $lb = bsearch($key,\@l);
push(@l,'-undef-');
print "bsearch(key=$key, l=[$lstr]) = l[lb=$lb] = $l[$lb]\n";
